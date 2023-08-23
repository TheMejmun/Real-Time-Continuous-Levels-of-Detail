#version 450

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec4 inWorldPos;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

const vec3 SUN_COLOR_LAB = vec3(87, 0, 22);
const vec3 SUN_POS = vec3(5, 5, -5);
const vec3 SHADOW_COLOR_LAB = vec3(11, 27, -44);
const vec3 CAMERA_POS = vec3(0, 0, -5);
const float LIGHT_POWER_FACTOR = 2.0;

mat3 RGB_TO_XYZ_MAT = mat3(
0.4124564f, 0.2126729f, 0.0193339f,
0.3575761f, 0.7151522f, 0.1191920f,
0.1804375f, 0.0721750f, 0.9503041f
);

mat3 XYZ_TO_RGB_MAT = mat3(
3.2404549f, -0.9692665f, 0.0556434f,
-1.5371389f, 1.8760110f, -0.2040258f,
-0.4985316f, 0.0415561f, 1.0572252f
);

const vec3 D65_XYZ = vec3(
0.950489f, 1.0f, 1.088840f
);

// https://en.wikipedia.org/wiki/CIELAB_color_space
const float LAB_DELTA = 6.0f / 29.0f;
const float LAB_DELTA_POW2 = LAB_DELTA * LAB_DELTA;
const float LAB_DELTA_POW2_MUL3 = 3.0f * LAB_DELTA_POW2;
const float LAB_DELTA_POW3 = LAB_DELTA_POW2 * LAB_DELTA;
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
    const float l = (lab.x + 16.0f) * ONEHUNDREDSIXTEENTH;
    const float x = D65_XYZ.x * lab_f_inv(l + (lab.y * FIVEHUNDREDTH));
    const float y = D65_XYZ.y * lab_f_inv(l);
    const float z = D65_XYZ.z * lab_f_inv(l - (lab.z * TWOHUNDREDTH));;
    return vec3(x, y, z);
}

void main() {
    vec3 N = normalize(inNormal);
    vec3 L = normalize(SUN_POS - inWorldPos.xyz);
    vec3 V = normalize(CAMERA_POS - inWorldPos.xyz);

    float brightness = dot(N, L);

    float lightFac = pow(clamp(brightness, 0, 1), LIGHT_POWER_FACTOR);
    float shadowFac = pow(clamp(-brightness, 0, 1), LIGHT_POWER_FACTOR);

    vec3 color = mix(SUN_COLOR_LAB, inColor, lightFac);
    color = mix(SHADOW_COLOR_LAB, color, shadowFac);
    color = inColor * brightness;

    outColor = vec4(xyzToRgb(labToXyz(color)), 1.0f);
}