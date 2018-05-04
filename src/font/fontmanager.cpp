/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2018                                                               *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/

#include <ghoul/font/fontmanager.h>

#include <ghoul/fmt.h>
#include <ghoul/filesystem/file.h>
#include <ghoul/filesystem/filesystem.h>
#include <ghoul/font/font.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/assert.h>
#include <ghoul/misc/crc32.h>
#include <ghoul/opengl/texture.h>

namespace ghoul::fontrendering {

FontManager::FontRegistrationException::FontRegistrationException(const std::string& msg)
    : RuntimeError(msg, "FontManager")
{}

FontManager::FontAccessException::FontAccessException(const std::string& msg)
    : RuntimeError(msg, "FontManager")
{}

FontManager::FontManager(glm::ivec3 atlasDimensions)
    : _textureAtlas(std::move(atlasDimensions))
    , _defaultCharacterSet({
        L' ', L'!', L'\\', L'"', L'#', L'$', L'%', L'&', L'\'', L'(',
        L')', L'*', L'+', L',', L'-', L'.', L'/', L'0', L'1', L'2',
        L'3', L'4', L'5', L'6', L'7', L'8', L'9', L':', L';', L'<',
        L'=', L'>', L'?', L'@', L'A', L'B', L'C', L'D', L'E', L'F',
        L'G', L'H', L'I', L'J', L'K', L'L', L'M', L'N', L'O', L'P',
        L'Q', L'R', L'S', L'T', L'U', L'V', L'W', L'X', L'Y', L'Z',
        L'[', L']', L'^', L'_', L'~', L'a', L'b', L'c', L'd', L'e',
        L'f', L'g', L'h', L'i', L'j', L'k', L'l', L'm', L'n', L'o',
        L'p', L'q', L'r', L's', L't', L'u', L'v', L'w', L'x', L'y',
        L'z', L'{', L'|', L'}'
    })
{}

unsigned int FontManager::registerFontPath(const std::string& fontName,
                                           const std::string& filePath)
{
    ghoul_assert(!fontName.empty(), "Fontname must not be empty");
    ghoul_assert(!filePath.empty(), "Filepath must not be empty");

    unsigned int hash = hashCRC32(fontName);
    auto it = _fontPaths.find(hash);
    if (it != _fontPaths.end()) {
        const std::string& registeredPath = it->second;

        if (registeredPath == filePath) {
            return hash;
        }
        else {
            throw FontRegistrationException(fmt::format(
                "Font '{}' was registered with path '{}' before. "
                    "Trying to register with path '{}' now",
                fontName,
                registeredPath,
                filePath
            ));
        }
    }
    _fontPaths[hash] = filePath;
    return hash;
}

std::shared_ptr<Font> FontManager::font(const std::string& name, float fontSize,
                                        Outline withOutline, LoadGlyphs loadGlyphs)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    unsigned int hash = hashCRC32(name);

    auto itPath = _fontPaths.find(hash);
    if (itPath == _fontPaths.end()) {
        // There is no hash registered for the current name, so it might be a local file
        if (FileSys.fileExists(name)) {
            hash = registerFontPath(name, name);
        }
        else {
            // The name has not neen previously registered and it is not a valid path
            throw FontAccessException(fmt::format(
                "Name '{}' does not name a valid font or file", name
            ));
        }
    }

    return font(hash, fontSize, withOutline, loadGlyphs);
}

std::shared_ptr<Font> FontManager::font(unsigned int hashName, float fontSize,
                                        Outline withOutline, LoadGlyphs loadGlyphs)
{
    auto itPath = _fontPaths.find(hashName);
    if (itPath == _fontPaths.end()) {
        throw FontAccessException(fmt::format(
            "Error retrieving font with hash '{}' for size '{}'", hashName, fontSize
        ));
    }

    auto fonts = _fonts.equal_range(hashName);
    for (auto it = fonts.first; it != fonts.second; ++it) {
        const float delta = 1e-6f; // Font sizes are 1-1000, so a delta of 1/e6 is fine
        if ((glm::abs(it->second->pointSize() - fontSize) < delta) &&
            it->second->hasOutline() == withOutline)
        {
            return it->second;
        }
    }

    std::shared_ptr<Font> f = std::make_shared<Font>(
        _fontPaths[hashName],
        fontSize,
        _textureAtlas,
        withOutline ? Font::Outline::Yes : Font::Outline::No
    );

    if (loadGlyphs) {
        f->loadGlyphs(_defaultCharacterSet);
    }

    _fonts.emplace(hashName, f);
    return f;
}

} // namespace ghoul::fontrendering
