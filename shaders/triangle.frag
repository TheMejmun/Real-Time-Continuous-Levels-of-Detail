#version 450

layout(location = 0) in vec3 inColor;

layout(location = 0) out vec4 outColor;

mat3 RGB_TO_XYZ_MAT = transpose(
    mat3(
        0.4124564f, 0.3575761f, 0.1804375f,
        0.2126729f, 0.7151522f, 0.0721750f,
        0.0193339f, 0.1191920f, 0.9503041f
    )
);

mat3 XYZ_TO_RGB_MAT = inverse(RGB_TO_XYZ_MAT);

const vec3 D65_XYZ = vec3(
    0.950489f, 1.0f, 1.08884f
);

// https://en.wikipedia.org/wiki/CIELAB_color_space
const float LAB_DELTA = 6.0f / 29.0f;
const float LAB_DELTA_POW2 = pow(LAB_DELTA, 2.0f);
const float LAB_DELTA_POW2_MUL3 = 3.0f * LAB_DELTA_POW2;
const float LAB_DELTA_POW3 = pow(LAB_DELTA, 3.0f);
const float THIRD = 1.0f / 3.0f;
const float FOUR_29TH = 4.0f / 29.0f;
const float FIVEHUNDREDTH = 1.0f / 500.0f;
const float TWOHUNDREDTH = 1.0f / 200.0f;
const float ONEHUNDREDSIXTEENTH = 1.0f / 116.0f;

float lab_f(const float t) {
    return t > LAB_DELTA_POW3 ?
        pow(t, THIRD) :
        ((t / LAB_DELTA_POW2_MUL3) + FOUR_29TH);
}

float lab_f_inv(const float t) {
    return t > LAB_DELTA ?
        pow(t, 3.0f) :
        (LAB_DELTA_POW2_MUL3 * (t - FOUR_29TH));
}

vec3 xyzToRgb(const vec3 xyz) {
    return XYZ_TO_RGB_MAT * xyz;
}

vec3 labToXyz(const vec3 lab) {
    vec3 labNorm = lab * 100.0f;
    const float l = (labNorm.x + 16.0f) * ONEHUNDREDSIXTEENTH;
    const float x = D65_XYZ.x * lab_f_inv(l + (labNorm.y * FIVEHUNDREDTH));
    const float y = D65_XYZ.y * lab_f_inv(l);
    const float z = D65_XYZ.z * lab_f_inv(l - (labNorm.z * TWOHUNDREDTH));;
    return vec3(x, y, z);
}

void main() {
    outColor = vec4(xyzToRgb(labToXyz(inColor)), 1.0f);
}