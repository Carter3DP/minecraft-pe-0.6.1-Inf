#ifndef NET_MINECRAFT_WORLD_PHYS__Vec3_H__
#define NET_MINECRAFT_WORLD_PHYS__Vec3_H__

//package net.minecraft.world.phys;

#include <cmath>
#include <string>
#include <sstream>

class Vec3
{
public:
	Vec3()
	:	x(0),
		y(0),
		z(0)
	{}

    Vec3(double x, double y, double z) {
        if (x == -0.0) x = 0.0;
        if (y == -0.0) y = 0.0;
        if (z == -0.0) z = 0.0;
        this->x = x;
        this->y = y;
        this->z = z;
    }

    Vec3* set(double x, double y, double z) {
        this->x = x;
        this->y = y;
        this->z = z;
        return this;
    }

	Vec3 operator+(const Vec3& rhs) const {
		return Vec3(x + rhs.x, y + rhs.y, z + rhs.z);
	}
	Vec3& operator+=(const Vec3& rhs) {
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	}

	Vec3 operator-(const Vec3& rhs) const {
		return Vec3(x - rhs.x, y - rhs.y, z - rhs.z);
	}
	Vec3& operator-=(const Vec3& rhs) {
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		return *this;
	}

    Vec3 operator*(double k) const {
        return Vec3(x * k, y * k, z * k);
    }
    Vec3& operator*=(double k) {
        x *= k;
        y *= k;
        z *= k;
        return *this;
    }

    Vec3 normalized() const {
        double dist = sqrt(x * x + y * y + z * z);
        if (dist < 0.0001) return Vec3();
        return Vec3(x / dist, y / dist, z / dist); // newTemp
    }

    double dot(const Vec3& p) const {
        return x * p.x + y * p.y + z * p.z;
    }

    Vec3 cross(const Vec3& p) const {
        return Vec3(y * p.z - z * p.y, z * p.x - x * p.z, x * p.y - y * p.x); // newTemp
    }

    Vec3 add(double x, double y, double z) const {
        return Vec3(this->x + x, this->y + y, this->z + z); // newTemp
    }
	Vec3& addSelf(double x, double y, double z) {
		this->x += x;
		this->y += y;
		this->z += z;
		return *this;
	}
    Vec3 sub(double x, double y, double z) const {
        return Vec3(this->x - x, this->y - y, this->z - z); // newTemp
    }
	Vec3& subSelf(double x, double y, double z) {
		this->x -= x;
		this->y -= y;
		this->z -= z;
		return *this;
	}

	void negate() {
		x = -x;
		y = -y;
		z = -z;
	}

	Vec3 negated() const {
		return Vec3(-x, -y, -z);
	}

    double distanceTo(const Vec3& p) const {
        double xd = p.x - x;
        double yd = p.y - y;
        double zd = p.z - z;
        return sqrt(xd * xd + yd * yd + zd * zd);
    }

    double distanceToSqr(const Vec3& p) const {
        double xd = p.x - x;
        double yd = p.y - y;
        double zd = p.z - z;
        return xd * xd + yd * yd + zd * zd;
    }

    double distanceToSqr(double x2, double y2, double z2) const {
        double xd = x2 - x;
        double yd = y2 - y;
        double zd = z2 - z;
        return xd * xd + yd * yd + zd * zd;
    }

    double length() const {
        return sqrt(x * x + y * y + z * z);
    }

    bool clipX(const Vec3& b, double xt, Vec3& result) const {
        double xd = b.x - x;
        double yd = b.y - y;
        double zd = b.z - z;

        if (xd * xd < 0.0000001) return false;

        double d = (xt - x) / xd;
        if (d < 0 || d > 1) return false;

		result.set(x + xd * d, y + yd * d, z + zd * d);
		return true;
    }

    bool clipY(const Vec3& b, double yt, Vec3& result) const {
        double xd = b.x - x;
        double yd = b.y - y;
        double zd = b.z - z;

        if (yd * yd < 0.0000001) return false;

        double d = (yt - y) / yd;
        if (d < 0 || d > 1) return false;

		result.set(x + xd * d, y + yd * d, z + zd * d);
		return true;
    }

    bool clipZ(const Vec3& b, double zt, Vec3& result) const {
        double xd = b.x - x;
        double yd = b.y - y;
        double zd = b.z - z;

        if (zd * zd < 0.0000001) return false;

        double d = (zt - z) / zd;
        if (d < 0 || d > 1) return false;

		result.set(x + xd * d, y + yd * d, z + zd * d);
		return true;
    }

    std::string toString() const {
		std::stringstream ss;
		ss << "Vec3(" << x << "," << y << "," << z << ")";
		return ss.str();
    }

    Vec3 lerp(const Vec3& v, float a) const {
        return Vec3(x + (v.x - x) * a, y + (v.y - y) * a, z + (v.z - z) * a); // newTemp
    }

    void xRot(double degs) {
        double cosv = cos(degs);
        double sinv = sin(degs);

        double xx = x;
        double yy = y * cosv + z * sinv;
        double zz = z * cosv - y * sinv;

        this->x = xx;
        this->y = yy;
        this->z = zz;
    }

    void yRot(double degs) {
        double cosv = cos(degs);
        double sinv = sin(degs);

        double xx = x * cosv + z * sinv;
        double yy = y;
        double zz = z * cosv - x * sinv;

        this->x = xx;
        this->y = yy;
        this->z = zz;
    }

    void zRot(double degs) {
        double cosv = cos(degs);
        double sinv = sin(degs);

        double xx = x * cosv + y * sinv;
        double yy = y * cosv - x * sinv;
        double zz = z;

        this->x = xx;
        this->y = yy;
        this->z = zz;
    }

	static Vec3 fromPolarXY(double angle, double radius) {
		return Vec3( radius * cos(angle), radius * sin(angle), 0 );
	}

	double x, y, z;
};

#endif /*NET_MINECRAFT_WORLD_PHYS__Vec3_H__*/
