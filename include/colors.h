//
// Created by Sam on 2023-04-09.
//

#ifndef REALTIME_CELL_COLLAPSE_COLORS_H
#define REALTIME_CELL_COLLAPSE_COLORS_H

#include <glm/glm.hpp>
#include <cmath>

namespace Colors {
    const glm::mat3 RGB_TO_XYZ_MAT = glm::transpose(glm::mat3{
            0.4124564, 0.3575761, 0.1804375,
            0.2126729, 0.7151522, 0.0721750,
            0.0193339, 0.1191920, 0.9503041
    });

    const glm::mat3 XYZ_TO_RGB_MAT = glm::inverse(RGB_TO_XYZ_MAT);

    const glm::vec3 D65_XYZ = {
            0.950489, 1.0, 1.08884
    };

    const glm::vec3 D50_XYZ = {
            0.964212, 1.0, 0.825188
    };

    const double SRGB_GAMMA = 2.4;

    inline double gammaEncode(double color) {
        return color <= 0.0031308
               ? color * 12.92
               : std::pow(color, 1.0 / SRGB_GAMMA) * 1.055 - 0.055;
    }

    inline double gammaDecode(double color) {
        return color <= 0.04045
               ? color / 12.92
               : std::pow((color + 0.055) / 1.055, SRGB_GAMMA);
    }

    inline glm::vec3 gammaEncode(const glm::vec3 &rgb) {
        return {
                gammaEncode(rgb.x),
                gammaEncode(rgb.y),
                gammaEncode(rgb.z),
        };
    }

    inline glm::vec3 gammaDecode(const glm::vec3 &srgb) {
        return {
                gammaDecode(srgb.x),
                gammaDecode(srgb.y),
                gammaDecode(srgb.z),
        };
    }

    inline glm::vec3 rgbToXyz(const glm::vec3 &rgb) {

        return RGB_TO_XYZ_MAT * rgb;
    }

    inline glm::vec3 xyzToRgb(const glm::vec3 &xyz) {
        return XYZ_TO_RGB_MAT * xyz;
    }

    // https://en.wikipedia.org/wiki/CIELAB_color_space
    const double _lab_delta = 6.0 / 29.0;
    const double _lab_delta_pow2 = std::pow(_lab_delta, 2.0);
    const double _lab_delta_pow2_mul3 = 3.0 * _lab_delta_pow2;
    const double _lab_delta_pow3 = std::pow(_lab_delta, 3.0);
    const double _third = 1.0 / 3.0;
    const double _four_29th = 4.0 / 29.0;

    inline double _lab_f(double t) {
        return t > _lab_delta_pow3 ?
               std::pow(t, _third) :
               ((t / _lab_delta_pow2_mul3) + _four_29th);
    }

    inline double _lab_f_inv(double t) {
        return t > _lab_delta ?
               std::pow(t, 3.0) :
               (_lab_delta_pow2_mul3 * (t - _four_29th));
    }

    glm::vec3 xyzToLab(const glm::vec3 &xyz) {
        double x = _lab_f(xyz.x / D65_XYZ.x);
        double y = _lab_f(xyz.y / D65_XYZ.y);
        double z = _lab_f(xyz.z / D65_XYZ.z);
        double lStar = 116.0 * y - 16.0;
        double aStar = 500.0 * (x - y);
        double bStar = 200.0 * (y - z);
        return {lStar, aStar, bStar};
    }

    glm::vec3 labToXyz(const glm::vec3 &lab) {
        double l = (lab.x + 16.0) / 116.0;
        double x = D65_XYZ.x * _lab_f_inv(l + (lab.y / 500.0));
        double y = D65_XYZ.y * _lab_f_inv(l);
        double z = D65_XYZ.z * _lab_f_inv(l - (lab.z / 200.0));;
        return {x, y, z};
    }
}

#endif //REALTIME_CELL_COLLAPSE_COLORS_H
