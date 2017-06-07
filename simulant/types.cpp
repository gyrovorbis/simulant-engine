//
//   Copyright (c) 2011-2017 Luke Benstead https://simulant-engine.appspot.com
//
//     This file is part of Simulant.
//
//     Simulant is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Simulant is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Simulant.  If not, see <http://www.gnu.org/licenses/>.
//

#include "types.h"
#include "utils/random.h"


namespace smlt {

const Vec3 Vec3::NEGATIVE_X = Vec3(-1, 0, 0);
const Vec3 Vec3::POSITIVE_X = Vec3(1, 0, 0);
const Vec3 Vec3::NEGATIVE_Y = Vec3(0, -1, 0);
const Vec3 Vec3::POSITIVE_Y = Vec3(0, 1, 0);
const Vec3 Vec3::POSITIVE_Z = Vec3(0, 0, 1);
const Vec3 Vec3::NEGATIVE_Z = Vec3(0, 0, -1);

Radians to_radians(const Degrees& degrees) {
    return Radians(degrees.value * PI_OVER_180);
}

Degrees to_degrees(const Radians& radians) {
    return Degrees(radians.value * PI_UNDER_180);
}



smlt::Quaternion operator-(const smlt::Quaternion& q) {
    return smlt::Quaternion(-q.x, -q.y, -q.z, -q.w);
}

std::ostream& operator<<(std::ostream& stream, const Vec2& vec) {
    stream << "(" << vec.x << "," << vec.y << ")";
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const Vec3& vec) {
    stream << "(" << vec.x << "," << vec.y << "," << vec.z << ")";
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const Vec4& vec) {
    stream << "(" << vec.x << "," << vec.y << "," << vec.z << "," << vec.w << ")";
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const Quaternion& quat) {
    stream << "(" << quat.x << "," << quat.y << "," << quat.z << "," << quat.w << ")";
    return stream;
}

Quaternion Quaternion::as_look_at(const Vec3& direction, const Vec3& up=Vec3(0, 1, 0)) {
    Mat4 lookat = Mat4::as_look_at(Vec3(), direction, up);
    Quaternion ret = glm::quat_cast(lookat);
    return ret;
}

Quaternion::Quaternion(Degrees pitch, Degrees yaw, Degrees roll) {
    Quaternion x = Quaternion(Vec3::POSITIVE_X, pitch);
    Quaternion y = Quaternion(Vec3::POSITIVE_Y, yaw);
    Quaternion z = Quaternion(Vec3::POSITIVE_Z, roll);

    *this = z * x * y;
}

Quaternion::Quaternion(const Vec3 &axis, const Degrees &degrees) {
    float rad = Radians(degrees).value * 0.5f;
    float scale	= sinf(rad);

    x = axis.x * scale;
    y = axis.y * scale;
    z = axis.z * scale;
    w = cosf(rad);

    normalize();
}

Quaternion::Quaternion(const Mat3& rot_matrix) {
    float m12 = rot_matrix[7];
    float m21 = rot_matrix[5];
    float m02 = rot_matrix[6];
    float m20 = rot_matrix[2];
    float m10 = rot_matrix[1];
    float m01 = rot_matrix[3];
    float m00 = rot_matrix[0];
    float m11 = rot_matrix[4];
    float m22 = rot_matrix[8];
    float t = m00 + m11 + m22;
    // we protect the division by s by ensuring that s>=1
    if (t >= 0) { // by w
        float s = sqrt(t + 1);
        w = 0.5 * s;
        s = 0.5 / s;
        x = (m21 - m12) * s;
        y = (m02 - m20) * s;
        z = (m10 - m01) * s;
    } else if ((m00 > m11) && (m00 > m22)) { // by x
        float s = sqrt(1 + m00 - m11 - m22);
        x = s * 0.5;
        s = 0.5 / s;
        y = (m10 + m01) * s;
        z = (m02 + m20) * s;
        w = (m21 - m12) * s;
    } else if (m11 > m22) { // by y
        float s = sqrt(1 + m11 - m00 - m22);
        y = s * 0.5;
        s = 0.5 / s;
        x = (m10 + m01) * s;
        z = (m21 + m12) * s;
        w = (m02 - m20) * s;
    } else { // by z
        float s = sqrt(1 + m22 - m00 - m11);
        z = s * 0.5;
        s = 0.5 / s;
        x = (m02 + m20) * s;
        y = (m21 + m12) * s;
        w = (m10 - m01) * s;
    }
}

Vec3 Quaternion::rotate_vector(const Vec3 &v) const {
    return (*this) * v;
}

Euler Quaternion::to_euler() const {
    auto ret = glm::eulerAngles(*this);
    return Euler(
                Degrees(Radians(ret.x)).value,
                Degrees(Radians(ret.y)).value,
                Degrees(Radians(ret.z)).value
                );
}

AxisAngle Quaternion::to_axis_angle() const {
    AxisAngle ret;
    ret.axis = axis();
    ret.angle = Degrees(glm::angle(*this));
    return ret;
}

Vec4 Mat4::operator*(const Vec4 &rhs) const {
    return glm::operator*(*this, rhs);
}

void Mat4::extract_rotation_and_translation(Quaternion& rotation, Vec3& translation) const {

    glm::vec3 scale;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::vec3 trans_tmp;

    glm::decompose(*this, scale, rotation, trans_tmp, skew, perspective);

    rotation = glm::conjugate(rotation);
    translation = Vec3(trans_tmp.x, trans_tmp.y, trans_tmp.z);
}

Mat4 Mat4::as_rotation_x(const Degrees &angle) {
    return glm::rotate(Mat4(), Radians(angle).value, glm::vec3(1, 0, 0));
}

Mat4 Mat4::as_rotation_y(const Degrees &angle) {
    return glm::rotate(Mat4(), Radians(angle).value, glm::vec3(0, 1, 0));
}

Mat4 Mat4::as_rotation_z(const Degrees &angle) {
    return glm::rotate(Mat4(), Radians(angle).value, glm::vec3(0, 0, 1));
}

Mat3 Mat3::from_rotation_x(const Degrees& angle) {
    return glm::mat3_cast(Quaternion(angle, Degrees(), Degrees()));
}

Mat3 Mat3::from_rotation_y(const Degrees &angle) {
    return glm::mat3_cast(Quaternion(Degrees(), angle, Degrees()));
}

Mat3 Mat3::from_rotation_z(const Degrees& angle) {
    return glm::mat3_cast(Quaternion(Degrees(), Degrees(), angle));
}

Mat3::Mat3(const Mat4 &rhs) {
    Quaternion q;
    Vec3 v;
    rhs.extract_rotation_and_translation(q, v);
    *this = glm::mat3_cast(q);
}

Vec3 Mat3::transform_vector(const Vec3 &v) const {
    auto tmp = glm::vec3(v.x, v.y, v.z) * (glm::mat3x3&) *this;
    return Vec3(tmp.x, tmp.y, tmp.z);
}

Mat4 Mat4::as_scaling(float s) {
    Mat4 ret = glm::scale(glm::mat4x4(), glm::vec3(s, s, s));
    return ret;
}

Mat4 Mat4::as_translation(const Vec3 &v) {
    Mat4 ret;
    ret = glm::translate((glm::mat4x4) ret, glm::vec3(v.x, v.y, v.z));
    return ret;
}

Plane Mat4::extract_plane(FrustumPlane plane) const {
    float t = 1.0f;
    Plane out;

    switch(plane) {
    case FRUSTUM_PLANE_RIGHT:
        out.n.x = (*this)[3] - (*this)[0];
        out.n.y = (*this)[7] - (*this)[4];
        out.n.z = (*this)[11] - (*this)[8];
        out.d = (*this)[15] - (*this)[12];
        break;
    case FRUSTUM_PLANE_LEFT:
        out.n.x = (*this)[3] + (*this)[0];
        out.n.y = (*this)[7] + (*this)[4];
        out.n.z = (*this)[11] + (*this)[8];
        out.d = (*this)[15] + (*this)[12];
        break;
    case FRUSTUM_PLANE_BOTTOM:
        out.n.x = (*this)[3] + (*this)[1];
        out.n.y = (*this)[7] + (*this)[5];
        out.n.z = (*this)[11] + (*this)[9];
        out.d = (*this)[15] + (*this)[13];
        break;
    case FRUSTUM_PLANE_TOP:
        out.n.x = (*this)[3] - (*this)[1];
        out.n.y = (*this)[7] - (*this)[5];
        out.n.z = (*this)[11] - (*this)[9];
        out.d = (*this)[15] - (*this)[13];
        break;
    case FRUSTUM_PLANE_FAR:
        out.n.x = (*this)[3] - (*this)[2];
        out.n.y = (*this)[7] - (*this)[6];
        out.n.z = (*this)[11] - (*this)[10];
        out.d = (*this)[15] - (*this)[14];
        break;
    case FRUSTUM_PLANE_NEAR:
        out.n.x = (*this)[3] + (*this)[2];
        out.n.y = (*this)[7] + (*this)[6];
        out.n.z = (*this)[11] + (*this)[10];
        out.d = (*this)[15] + (*this)[14];
        break;
    default:
        assert(0 && "Invalid plane index");
    }

    t = sqrtf(out.n.x * out.n.x +
              out.n.y * out.n.y +
              out.n.z * out.n.z);
    out.n.x /= t;
    out.n.y /= t;
    out.n.z /= t;
    out.d /= t;

    return out;
}

Mat4 Mat4::as_look_at(const Vec3& eye, const Vec3& target, const Vec3& up) {
    /* If the up vector is parellel to the view vector then we swap the up Y/Z axis */

    float d = up.dot((target - eye));

    Mat4 ret = glm::lookAt(
        glm::vec3(eye.x, eye.y, eye.z),
        glm::vec3(target.x, target.y, target.z),
        (almost_equal(d*d, 1.0f)) ? glm::vec3(up.x, up.z, up.y) : glm::vec3(up.x, up.y, up.z)
    );
    return ret;
}

Degrees::Degrees(const Radians &rhs):
    value(rhs.value * PI_UNDER_180) {

}

Radians::Radians(const Degrees &rhs):
    value(rhs.value * PI_OVER_180) {
}

uint32_t vertex_attribute_size(VertexAttribute attr) {
    switch(attr) {
    case VERTEX_ATTRIBUTE_NONE: return 0;
    case VERTEX_ATTRIBUTE_2F: return sizeof(float) * 2;
    case VERTEX_ATTRIBUTE_3F:  return sizeof(float) * 3;
    case VERTEX_ATTRIBUTE_4F: return sizeof(float) * 4;
    default:
        assert(0 && "Invalid attribute specified");
    }
}

VertexSpecification::VertexSpecification(VertexAttribute position, VertexAttribute normal, VertexAttribute texcoord0,
        VertexAttribute texcoord1, VertexAttribute texcoord2, VertexAttribute texcoord3, VertexAttribute texcoord4,
        VertexAttribute texcoord5, VertexAttribute texcoord6, VertexAttribute texcoord7,
        VertexAttribute diffuse, VertexAttribute specular):
    position_attribute(position),
    normal_attribute(normal),
    texcoord0_attribute(texcoord0),
    texcoord1_attribute(texcoord1),
    texcoord2_attribute(texcoord2),
    texcoord3_attribute(texcoord3),
    texcoord4_attribute(texcoord4),
    texcoord5_attribute(texcoord5),
    texcoord6_attribute(texcoord6),
    texcoord7_attribute(texcoord7),
    diffuse_attribute(diffuse),
    specular_attribute(specular) {

    recalc_stride();
}

bool VertexSpecification::has_texcoordX(uint8_t which) const {
    static const std::array<bool (VertexSpecification::*)() const, 8> LOOKUPS = {{
            &VertexSpecification::has_texcoord0,
            &VertexSpecification::has_texcoord1,
            &VertexSpecification::has_texcoord2,
            &VertexSpecification::has_texcoord3,
            &VertexSpecification::has_texcoord4,
            &VertexSpecification::has_texcoord5,
            &VertexSpecification::has_texcoord6,
            &VertexSpecification::has_texcoord7
    }};

    assert(which < MAX_TEXTURE_UNITS);

    return std::bind(LOOKUPS[which], this)();
}

const VertexAttribute VertexSpecification::texcoordX_attribute(uint8_t which) const {
    assert(which < MAX_TEXTURE_UNITS);

    switch(which) {
    case 0: return texcoord0_attribute;
    case 1: return texcoord1_attribute;
    case 2: return texcoord2_attribute;
    case 3: return texcoord3_attribute;
    case 4: return texcoord4_attribute;
    case 5: return texcoord5_attribute;
    case 6: return texcoord6_attribute;
    case 7: return texcoord7_attribute;
    default:
        throw std::out_of_range("Invalid texcoord");
    }
}

void VertexSpecification::recalc_stride() {
    stride_ = (
                vertex_attribute_size(position_attribute) +
                vertex_attribute_size(normal_attribute) +
                vertex_attribute_size(texcoord0_attribute) +
                vertex_attribute_size(texcoord1_attribute) +
                vertex_attribute_size(texcoord2_attribute) +
                vertex_attribute_size(texcoord3_attribute) +
                vertex_attribute_size(texcoord4_attribute) +
                vertex_attribute_size(texcoord5_attribute) +
                vertex_attribute_size(texcoord6_attribute) +
                vertex_attribute_size(texcoord7_attribute) +
                vertex_attribute_size(diffuse_attribute) +
                vertex_attribute_size(specular_attribute)
                );
}

uint32_t VertexSpecification::position_offset(bool check) const {
    if(check && !has_positions()) { throw std::logic_error("No such attribute"); }
    return 0;
}

uint32_t VertexSpecification::normal_offset(bool check) const {
    if(check && !has_normals()) { throw std::logic_error("No such attribute"); }
    return vertex_attribute_size(position_attribute);
}

uint32_t VertexSpecification::texcoord0_offset(bool check) const {
    if(check && !has_texcoord0()) { throw std::logic_error("No such attribute"); }
    return normal_offset(false) + vertex_attribute_size(normal_attribute);
}

uint32_t VertexSpecification::texcoord1_offset(bool check) const {
    if(check && !has_texcoord1()) { throw std::logic_error("No such attribute"); }
    return texcoord0_offset(false) + vertex_attribute_size(texcoord0_attribute);
}

uint32_t VertexSpecification::texcoord2_offset(bool check) const {
    if(check && !has_texcoord2()) { throw std::logic_error("No such attribute"); }
    return texcoord1_offset(false) + vertex_attribute_size(texcoord1_attribute);
}

uint32_t VertexSpecification::texcoord3_offset(bool check) const {
    if(check && !has_texcoord3()) { throw std::logic_error("No such attribute"); }
    return texcoord2_offset(false) + vertex_attribute_size(texcoord2_attribute);
}

uint32_t VertexSpecification::texcoord4_offset(bool check) const {
    if(check && !has_texcoord4()) { throw std::logic_error("No such attribute"); }
    return texcoord3_offset(false) + vertex_attribute_size(texcoord3_attribute);
}

uint32_t VertexSpecification::texcoord5_offset(bool check) const {
    if(check && !has_texcoord5()) { throw std::logic_error("No such attribute"); }
    return texcoord4_offset(false) + vertex_attribute_size(texcoord4_attribute);
}

uint32_t VertexSpecification::texcoord6_offset(bool check) const {
    if(check && !has_texcoord6()) { throw std::logic_error("No such attribute"); }
    return texcoord5_offset(false) + vertex_attribute_size(texcoord5_attribute);
}

uint32_t VertexSpecification::texcoord7_offset(bool check) const {
    if(check && !has_texcoord7()) { throw std::logic_error("No such attribute"); }
    return texcoord6_offset(false) + vertex_attribute_size(texcoord6_attribute);
}

uint32_t VertexSpecification::texcoordX_offset(uint8_t which, bool check) const {
    static const std::array<uint32_t (VertexSpecification::*)(bool) const, 8> LOOKUPS = {
            &VertexSpecification::texcoord0_offset,
            &VertexSpecification::texcoord1_offset,
            &VertexSpecification::texcoord2_offset,
            &VertexSpecification::texcoord3_offset,
            &VertexSpecification::texcoord4_offset,
            &VertexSpecification::texcoord5_offset,
            &VertexSpecification::texcoord6_offset,
            &VertexSpecification::texcoord7_offset
    };

    assert(which < MAX_TEXTURE_UNITS);

    return std::bind(LOOKUPS[which], this, check)();
}

uint32_t VertexSpecification::diffuse_offset(bool check) const {
    if(check && !has_diffuse()) { throw std::logic_error("No such attribute"); }
    return texcoord3_offset(false) + vertex_attribute_size(texcoord3_attribute);
}

uint32_t VertexSpecification::specular_offset(bool check) const {
    if(check && !has_specular()) { throw std::logic_error("No such attribute"); }
    return diffuse_offset(false) + vertex_attribute_size(diffuse_attribute);
}

float smlt::math::lerp(float a, float b, float t) {
    t = (t > 1.0) ? 1.0 : t;
    t = (t < 0.0) ? 0.0 : t;
    return a + ((b - a) * t);
}

Radians math::lerp_angle(Radians a, Radians b, float t) {
    float from_angle = std::fmod(a.value + TWO_PI, TWO_PI);
    float to_angle = std::fmod(b.value + TWO_PI, TWO_PI);

    auto diff = std::fabs(from_angle - to_angle);

    if(diff < PI) {
        return Radians(lerp(from_angle, to_angle, t));
    } else {
        if(from_angle > to_angle) {
            from_angle = from_angle - TWO_PI;
            return Radians(lerp(from_angle, to_angle, t));
        } else {
            to_angle = to_angle - TWO_PI;
            return Radians(lerp(from_angle, to_angle, t));
        }
    }
}

Degrees math::lerp_angle(Degrees a, Degrees b, float t) {
    return Degrees(lerp_angle(Radians(a), Radians(b), t));
}

Vec3 Plane::project(const Vec3 &p) {
    /* Not sure if there is a more direct way to do this in GLM.. */

    return p - (n.dot(p) + d) * n;
}

float Plane::distance_to(const Vec3 &p) {
    float k1 = d;
    float k2 = (n.x * p.x) + (n.y * p.y) + (n.z * p.z);
    return k2 - k1;
}

PlaneClassification Plane::classify_point(const Vec3 &p) const {
    /* This function will determine if a point is on, in front of, or behind*/
    /* the plane.  First we store the dot product of the plane and the point.*/
    auto distance = n.x * p.x + n.y * p.y + n.z * p.z + d;

    /* Simply put if the dot product is greater than 0 then it is infront of it.*/
    /* If it is less than 0 then it is behind it.  And if it is 0 then it is on it.*/
    if(distance > std::numeric_limits<float>::epsilon()) return PlaneClassification(1);
    if(distance < -std::numeric_limits<float>::epsilon()) return PlaneClassification(-1);

    return PlaneClassification(0);
}

smlt::optional<Vec3> Plane::intersect_planes(const Plane &p1, const Plane &p2, const Plane &p3) {
    const Vec3& n1 = p1.n;
    const Vec3& n2 = p2.n;
    const Vec3& n3 = p3.n;

    auto cross = n2.cross(n3);
    auto denom = n1.dot(cross);

    if(almost_equal(denom, 0.0f)) {
        return smlt::optional<Vec3>();
    }

    auto r1 = n2.cross(n3);
    auto r2 = n3.cross(n1);
    auto r3 = n1.cross(n2);

    r1 *= -p1.d;
    r2 *= p2.d;
    r3 *= p3.d;

    auto ret = r1 - r2 - r3;

    ret *= 1.0 / denom;

    return smlt::optional<Vec3>(std::move(ret));
}

bool Ray::intersects_aabb(const AABB &aabb) const {
    //http://gamedev.stackexchange.com/a/18459/15125
    Vec3 rdir = this->dir.normalized();
    Vec3 dirfrac(1.0 / rdir.x, 1.0 / rdir.y, 1.0 / rdir.z);

    float t1 = (aabb.min().x - start.x) * dirfrac.x;
    float t2 = (aabb.max().x - start.x) * dirfrac.x;
    float t3 = (aabb.min().y - start.y) * dirfrac.y;
    float t4 = (aabb.max().y - start.y) * dirfrac.y;
    float t5 = (aabb.min().z - start.z) * dirfrac.z;
    float t6 = (aabb.max().z - start.z) * dirfrac.z;

    float tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
    float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));

    // if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behind us
    if(tmax < 0) {
        return false;
    }

    // if tmin > tmax, ray doesn't intersect AABB
    if (tmin > tmax) {
        return false;
    }

    return false;
}

bool Ray::intersects_triangle(const Vec3 &v1, const Vec3 &v2, const Vec3 &v3, Vec3 *intersection, Vec3 *normal, float *distance) const {
    Vec3 hit;
    bool ret = glm::intersectLineTriangle(
                (const glm::vec3&) start,
                (const glm::vec3&) dir,
                (const glm::vec3&) v1,
                (const glm::vec3&) v2,
                (const glm::vec3&) v3,
                (glm::vec3&) hit
                );

    if(ret) {
        if(intersection) *intersection = hit;
        if(normal) {
            *normal = (v2 - v1).cross(v3 - v1).normalized();
        }

        if(distance) {
            *distance = (hit - start).length();
        }
    }

    return ret;
}

AABB::AABB(const Vec3 &min, const Vec3 &max) {
    set_min(min);
    set_max(max);
    rebuild_corners();
}

AABB::AABB(const Vec3 &centre, float width) {
    set_min(centre - Vec3(width * 0.5, width * 0.5, width * 0.5));
    set_max(centre + Vec3(width * 0.5, width * 0.5, width * 0.5));

    rebuild_corners();
}

AABB::AABB(const Vec3 &centre, float xsize, float ysize, float zsize) {
    set_min(centre - Vec3(xsize * 0.5, ysize * 0.5, zsize * 0.5));
    set_max(centre + Vec3(xsize * 0.5, ysize * 0.5, zsize * 0.5));

    rebuild_corners();
}

AABB::AABB(const Vec3 *vertices, const std::size_t count) {
    if(count == 0) {
        set_min(Vec3());
        set_max(Vec3());
        rebuild_corners();
        return;
    }

    float minx = std::numeric_limits<float>::max();
    float miny = std::numeric_limits<float>::max();
    float minz = std::numeric_limits<float>::max();

    float maxx = std::numeric_limits<float>::lowest();
    float maxy = std::numeric_limits<float>::lowest();
    float maxz = std::numeric_limits<float>::lowest();

    for(uint32_t i = 0; i < count; ++i) {
        if(vertices[i].x < minx) minx = vertices[i].x;
        if(vertices[i].y < miny) miny = vertices[i].y;
        if(vertices[i].z < minz) minz = vertices[i].z;

        if(vertices[i].x > maxx) maxx = vertices[i].x;
        if(vertices[i].y > maxy) maxy = vertices[i].y;
        if(vertices[i].z > maxz) maxz = vertices[i].z;
    }

    set_min(Vec3(minx, miny, minz));
    set_max(Vec3(maxx, maxy, maxz));

    rebuild_corners();
}

}
