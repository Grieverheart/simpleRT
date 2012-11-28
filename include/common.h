#ifndef RTCOMMON_H
#define RTCOMMON_H

#ifndef M_PI
#define M_PI 3.141593f
#endif

#define GOLDEN_ANGLE 2.399963f

#define GLM_SWIZZLE

typedef unsigned int uint;
typedef unsigned char uchar;

struct colorRGBF{
	colorRGBF(void): r(0.0f), g(0.0f), b(0.0f) {};
	explicit colorRGBF(int ri, int gi, int bi): r(ri / 255.0f), g(gi / 255.0f), b(bi / 255.0f) {};
	explicit colorRGBF(float ri, float gi, float bi): r(ri), g(gi), b(bi) {};
	explicit colorRGBF(int ci): r(ci / 255.0f), g(ci / 255.0f), b(ci / 255.0f) {};
	explicit colorRGBF(float ci): r(ci), g(ci), b(ci) {};
	colorRGBF &operator*=(colorRGBF const &rhs){
		this->r *= rhs.r;
		this->g *= rhs.g;
		this->b *= rhs.b;
		return *this;
	}
	const colorRGBF operator*(colorRGBF const &other)const{
		colorRGBF color(*this);
		color *= other;
		return color;
	}
	colorRGBF &operator+=(colorRGBF const &rhs){
		this->r += rhs.r;
		this->g += rhs.g;
		this->b += rhs.b;
		return *this;
	}
	const colorRGBF operator+(colorRGBF const &other)const{
		colorRGBF color(*this);
		color += other;
		return color;
	}
	colorRGBF &operator*=(float a){
		this->r *= a;
		this->g *= a;
		this->b *= a;
		return *this;
	}
	const colorRGBF operator*(float a){
		colorRGBF color(*this);
		color *= a;
		return color;
	}
	friend const colorRGBF operator*(float a, colorRGBF const &other){
		colorRGBF color(other);
		color *= a;
		return color;
	}
	float power(void)const{
		return (r + g + b) / 3.0f;
	}
	float r, g, b;
};

#endif
