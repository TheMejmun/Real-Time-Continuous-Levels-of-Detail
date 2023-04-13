//
// Created by Sam on 2023-04-09.
//

#ifndef REALTIME_CELL_COLLAPSE_COLORS_H
#define REALTIME_CELL_COLLAPSE_COLORS_H

#include <glm/glm.hpp>
#include <cmath>
#include <string>

// TODO implement CIELUV and CIELCh_uv

class Color {
public:
    static Color fromRGB(const double &r, const double &g, const double &b);

    static Color fromRGB(const glm::vec3 &rgb);

    static Color fromSRGB(const double &r, const double &g, const double &b);

    static Color fromSRGB(const glm::vec3 &srgb);

    static Color fromXYZ(const double &x, const double &y, const double &z);

    static Color fromXYZ(const glm::vec3 &xyz);

    static Color fromLAB(const double &lStar, const double &aStar, const double &bStar);

    static Color fromLAB(const glm::vec3 &lab);

    glm::vec3 getRGB();

    glm::vec3 getSRGB();

    glm::vec3 getXYZ();

    glm::vec3 getLAB();

    Color setLumaXYZ(const double &luma);

    Color setLumaLab(const double &luma);

    std::string toString();

private:
    explicit Color(glm::vec3 xyz);

    static double gammaEncode(const double &color);

    static double gammaDecode(const double &color);

    static glm::vec3 gammaEncode(const glm::vec3 &rgb);

    static glm::vec3 gammaDecode(const glm::vec3 &srgb);

    static glm::vec3 rgbToXyz(const glm::vec3 &rgb);

    static glm::vec3 xyzToRgb(const glm::vec3 &xyz);

    static double _lab_f(const double &t);

    static double _lab_f_inv(const double &t);

    static glm::vec3 xyzToLab(const glm::vec3 &xyz);

    static glm::vec3 labToXyz(const glm::vec3 &lab);

    static glm::vec3 rgbSetLuma(const glm::vec3 &rgb, const double &luma);

    glm::vec3 xyz = {0.0f, 0.0f, 0.0f};
};

#endif //REALTIME_CELL_COLLAPSE_COLORS_H
