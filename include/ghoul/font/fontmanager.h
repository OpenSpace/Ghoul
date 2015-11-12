/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2015                                                               *
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

#ifndef __FONTMANAGER_H__
#define __FONTMANAGER_H__

#include <ghoul/font/font.h>
#include <ghoul/glm.h>
#include <ghoul/opengl/textureatlas.h>

#include <map>
#include <string>
#include <vector>

namespace ghoul {
namespace fontrendering {
    
/**
 * This class manages different Font objects, stores them in a TextureAtlas and provides
 * access to these fonts based a user-defined, unique name. The paths to fonts must be
 * registered (#registerFontPath) with a name before the fonts can be accessed (#font).
 * Each registered font can give rise to one Font class for each requested fontSize. When
 * each Font is created, it is, on default, initialized with a default set of glyphs and
 * has an outline with a default outline thickness.
 */
class FontManager {
public:
    /**
     * The constructor that initializes the TextureAtlas. This means that this constructor
     * requires a valid OpenGL context.
     * \param atlasDimensions The dimensions of the TextureAtlas that is used as storage
     * for all fonts that are loaded through this FontManager. See the TextureAtlas
     * constructor documentation for limitations on the sizes
     */
    FontManager(glm::ivec3 atlasDimensions = glm::ivec3(512, 512, 1));
    
    /**
     * Copy-constructor for the FontManager that performs a deep-copy on all its settings.
     * After the construction, both FontManagers can be used independently.
     * \param rhs The source FontManager whose settings are copied
     */
    FontManager(const FontManager& rhs);
    
    /**
     * Move constructor that moves all settings and leaves the incoming FontManager in an
     * undefined state.
     * \param rhs The incoming FontManager from which the settings are moved
     */
    FontManager(FontManager&& rhs);
    
    /**
     * Default destructor that will delete all of the Fonts that have been created. All
     * Fonts that have been created using this FontManager will be deleted.
     */
    ~FontManager();
    
    /**
     * Assignment operator that performs a deep-copy of all of the incoming FontManager's
     * settings, leaving two independent FontManagers
     * \param rhs The incoming FontManager whose settings are deep-copied
     */
    FontManager& operator=(const FontManager& rhs);
    
    /**
     * Movement operator that moves all settings from the incoming FontManager, leaving it
     * in an undefined state.
     * \param rhs The incoming FontManager from which the settings are moved
     */
    FontManager& operator=(FontManager&& rhs);

    /**
     * Registers a user-defined <code>fontName</code> to an absolute
     * <code>filePath</code>. This function does not check whether the file exists, or is
     * accessible. If either of these is the case, subsequent #font calls will report an
     * error and failt to produce a Font. The <code>fontName</code> cannot have been
     * assigned to a different <code>filePath</code> or an error is reported.
     * \param fontName The user-defined name under which this font is registered
     * \param filePath The filepath for this registered Font
     * \return <code>true</code> if the Font was correctly registered, <code>false</code>
     * otherwise
     */
    bool registerFontPath(const std::string& fontName, const std::string& filePath);
    
    /**
     * Registers a user-defined <code>fontName</code> to an absolute
     * <code>filePath</code>. This function does not check whether the file exists, or is
     * accessible. If either of these is the case, subsequent #font calls will report an
     * error and failt to produce a Font. The <code>fontName</code> cannot have been
     * assigned to a different <code>filePath</code> or an error is reported. This method
     * will also create a hashed version of the <code>fontName</code> that can be used in
     * calls to the #font method for a more efficient lookup.
     * \param fontName The user-defined name under which this font is registered
     * \param filePath The filepath for this registered Font
     * \param hashedName Output variable into which the hashed representation of the
     * <code>fontName</code> will be stored. Only if the method succeeds will this
     * parameter be modified
     * \return <code>true</code> if the Font was correctly registered, <code>false</code>
     * otherwise
     */
    bool registerFontPath(const std::string& fontName,
                          const std::string& filePath, unsigned int& hashedName);

    /**
     * Retrieves the Font with the name <code>name</code>, which must have been previously
     * registered (#registerFontPath). If this is the first call to this function for a 
     * specific combination of <code>name</code> and <code>fontSize</code> the Font is
     * initialized. In this case, the <code>loadGlyphs</code> parameter determines whether
     * the Font object will be initialized with a common set of English glyphs. Otherwise,
     * the function will return immediately with the correct Font object. If the
     * <code>name</code> does not name a registered Font or the registered path does not
     * exist, a <code>nullptr</code> is returned.
     * \param name User-defined name for the Font that is to be retrieved
     * \param fontSize The font size (in pt) for the Font
     * \param withOutline If this parameter is <code>true</code> the created Font will
     * contain outlines as well as the base Font
     * \param loadGlyphs If <code>true</code>, the first initialization of the Font will
     * also preload a set of commonly used glyphs
     * \return Returns a usable and initialized Font object, or <code>nullptr</code> if an
     * error occurred
     */
    Font* font(const std::string& name, float fontSize, bool withOutline = true,
               bool loadGlyphs = true);

    /**
     * Retrieves the Font with the hashed name <code>hashName</code>, which must have been
     * previously registered (#registerFontPath). If this is the first call to this
     * function for a specific combination of <code>hashName</code> and
     * <code>fontSize</code> the Font is initialized. In this case, the\
     * <code>loadGlyphs</code> parameter determines whether the Font object will be
     * initialized with a common set of English glyphs. Otherwise, the function will
     * return immediately with the correct Font object. If the <code>name</code> does not
     * name a registered Font or the registered path does not exist, a
     * <code>nullptr</code> is returned.
     * \param hashName A hashed name of the font that is to be retrieved
     * \param fontSize The font size (in pt) for the Font
     * \param withOutline If this parameter is <code>true</code> the created Font will
     * contain outlines as well as the base Font
     * \param loadGlyphs If <code>true</code>, the first initialization of the Font will
     * also preload a set of commonly used glyphs
     * \return Returns a usable and initialized Font object, or <code>nullptr</code> if an
     * error occurred
     */
    Font* font(unsigned int hashName, float fontSize, bool withOutline = true,
               bool loadGlyphs = true);
    
private:
    /// The TextureAtlas that is used to store all glyphs for all registered Font objects
    ghoul::opengl::TextureAtlas _textureAtlas;
    
    /// The map that is used to retrieve previously created Font objects.
    std::multimap<unsigned int, Font*> _fonts;
    
    /// The map that correlates the hashed names with the file paths for the fonts
    std::map<unsigned int, std::string> _fontPaths;
    
    /// The default set of glyphs that are loaded when a new Font is initialized
    std::vector<wchar_t> _defaultCharacterSet;
};
    
} // namespace fontrendering
} // namespace ghoul

#endif // __FONTMANAGER_H__
