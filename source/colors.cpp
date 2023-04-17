//
// Created by Sam on 2023-04-13.
//
#include "colors.h"
#include <sstream>
#include <iomanip>
#include <random>

glm::vec3 White = {1.0f, 1.0f, 1.0f};

const glm::mat3 RGB_TO_XYZ_MAT = glm::mat3{
        0.4124564, 0.2126729, 0.0193339,
        0.3575761, 0.7151522, 0.1191920,
        0.1804375, 0.0721750, 0.9503041
};

const glm::mat3 XYZ_TO_RGB_MAT = glm::mat3{
        3.2404549, -0.9692665, 0.0556434,
        -1.5371389, 1.8760110, -0.2040258,
        -0.4985316, 0.0415561, 1.0572252,
};

const glm::vec3 D65_XYZ = {
        0.950489, 1.0, 1.088840
};

const glm::vec3 D50_XYZ = {
        0.964212, 1.0, 0.825188
};

constexpr double SRGB_GAMMA = 2.4;

// https://en.wikipedia.org/wiki/CIELAB_color_space
constexpr double LAB_DELTA = 6.0 / 29.0;
constexpr double LAB_DELTA_POW2 = LAB_DELTA * LAB_DELTA;
constexpr double LAB_DELTA_POW2_MUL3 = 3.0 * LAB_DELTA_POW2;
constexpr double LAB_DELTA_POW3 = LAB_DELTA_POW2 * LAB_DELTA;
constexpr double THIRD = 1.0 / 3.0;
constexpr double FOUR_29TH = 4.0 / 29.0;
constexpr double FIVEHUNDREDTH = 1.0 / 500.0;
constexpr double TWOHUNDREDTH = 1.0 / 200.0;
constexpr double ONEHUNDREDSIXTEENTH = 1.0 / 116.0;

Color Color::random() {
    std::random_device device;
    std::default_random_engine engine(device());
    std::uniform_real_distribution<float> distribution(0.0f, 1.0f);

    // From RGB, to avoid because all colors 0 .. 1 are displayable
    return Color::fromRGB(distribution(engine), distribution(engine), distribution(engine));
}

Color::Color(glm::vec3 xyz) {
    this->xyz = xyz;
}

Color Color::fromRGB(const double &r, const double &g, const double &b) {
    return fromRGB({r, g, b});
}

Color Color::fromRGB(const glm::vec3 &rgb) {
    return Color(rgbToXyz(rgb));
}

Color Color::fromSRGB(const double &r, const double &g, const double &b) {
    return fromSRGB({r, g, b});
}

Color Color::fromSRGB(const glm::vec3 &srgb) {
    return Color(rgbToXyz(gammaDecode(srgb)));
}

Color Color::fromXYZ(const double &x, const double &y, const double &z) {
    return fromXYZ({x, y, z});
}

Color Color::fromXYZ(const glm::vec3 &xyz) {
    return Color(xyz);
}

Color Color::fromLAB(const double &lStar, const double &aStar, const double &bStar) {
    return fromLAB({lStar, aStar, bStar});
}

Color Color::fromLAB(const glm::vec3 &lab) {
    return Color(labToXyz(lab * 100.0f));
}

glm::vec3 Color::getRGB() {
    return xyzToRgb(this->xyz);
}

glm::vec3 Color::getSRGB() {
    return gammaEncode(xyzToRgb(this->xyz));
}

glm::vec3 Color::getXYZ() {
    return this->xyz;
}

glm::vec3 Color::getLAB() {
    return xyzToLab(this->xyz) / 100.0f;
}

Color Color::setLumaXYZ(const double &luma) {
    this->xyz.y = static_cast<float>(luma);
    return *this;
}

Color Color::setLumaLab(const double &luma) {
    auto lab = xyzToLab(this->xyz);
    lab.x = static_cast<float>(luma * 100);
    this->xyz = labToXyz(lab);
    return *this;
}

std::string Color::toString() {
    auto rgb = getRGB();
    auto srgb = getSRGB();
    auto lab = getLAB();

    std::stringstream stream;
    stream << std::fixed << std::setprecision(4);
    stream << "RGB:\t(r: " << rgb.r << ", g: " << rgb.g << ", b: " << rgb.b << ")" << std::endl;
    stream << "SRGB:\t(r: " << srgb.r << ", g: " << srgb.g << ", b: " << srgb.b << ")" << std::endl;
    stream << "XYZ:\t(x: " << xyz.r << ", y: " << xyz.g << ", z: " << xyz.b << ")" << std::endl;
    stream << "L*a*b*:\t(l: " << lab.r << ", a: " << lab.g << ", b: " << lab.b << ")";

    return stream.str();
}

double Color::gammaEncode(const double &color) {
    return color <= 0.0031308
           ? color * 12.92
           : std::pow(color, 1.0 / SRGB_GAMMA) * 1.055 - 0.055;
}

double Color::gammaDecode(const double &color) {
    return color <= 0.04045
           ? color / 12.92
           : std::pow((color + 0.055) / 1.055, SRGB_GAMMA);
}

glm::vec3 Color::gammaEncode(const glm::vec3 &rgb) {
    return {
            gammaEncode(rgb.x),
            gammaEncode(rgb.y),
            gammaEncode(rgb.z),
    };
}

glm::vec3 Color::gammaDecode(const glm::vec3 &srgb) {
    return {
            gammaDecode(srgb.x),
            gammaDecode(srgb.y),
            gammaDecode(srgb.z),
    };
}

glm::vec3 Color::rgbToXyz(const glm::vec3 &rgb) {
    return RGB_TO_XYZ_MAT * rgb;
}

glm::vec3 Color::xyzToRgb(const glm::vec3 &xyz) {
    return XYZ_TO_RGB_MAT * xyz;
}

double Color::_lab_f(const double &t) {
    return t > LAB_DELTA_POW3 ?
           std::pow(t, THIRD) :
           ((t / LAB_DELTA_POW2_MUL3) + FOUR_29TH);
}

double Color::_lab_f_inv(const double &t) {
    return t > LAB_DELTA ?
           std::pow(t, 3.0) :
           (LAB_DELTA_POW2_MUL3 * (t - FOUR_29TH));
}

glm::vec3 Color::xyzToLab(const glm::vec3 &xyz) {
    const double x = _lab_f(xyz.x / D65_XYZ.x);
    const double y = _lab_f(xyz.y / D65_XYZ.y);
    const double z = _lab_f(xyz.z / D65_XYZ.z);
    const double lStar = 116.0 * y - 16.0;
    const double aStar = 500.0 * (x - y);
    const double bStar = 200.0 * (y - z);
    return {lStar, aStar, bStar};
}

glm::vec3 Color::labToXyz(const glm::vec3 &lab) {
    const double l = (lab.x + 16.0) * ONEHUNDREDSIXTEENTH;
    const double x = D65_XYZ.x * _lab_f_inv(l + (lab.y * FIVEHUNDREDTH));
    const double y = D65_XYZ.y * _lab_f_inv(l);
    const double z = D65_XYZ.z * _lab_f_inv(l - (lab.z * TWOHUNDREDTH));;
    return {x, y, z};
}

glm::vec3 Color::rgbSetLuma(const glm::vec3 &rgb, const double &luma) {
    glm::vec3 lab = xyzToLab(rgbToXyz(rgb));
    lab.x = static_cast<float>(luma * 100);
    return xyzToRgb(labToXyz(lab));
}