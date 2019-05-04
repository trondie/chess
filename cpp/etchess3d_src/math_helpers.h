#ifndef _MATH_HELPERS_H_
#define _MATH_HELPERS_H_

#define _USE_MATH_DEFINES
#include <math.h>

/**
	Template class for 2-component vector storage and manipulation.
	@ingroup Vecmath
*/
template <typename T> class myvec2t
{
public:
	/**
		Vector components, represented as a union between
		two values (x and y), and an array of T.
	*/
	union
	{
		struct { T x, y; };
		T coord[2];
	};

	/**
		Empty default constructor, performs no initialization
	*/
	myvec2t()
	{
	}

	/**
		Constructor, initializes the vector components
		@param x The initial x-component value of the constructed vector
		@param y The initial y-component value of the constructed vector
		
	*/
	myvec2t(T x, T y)
	{
		this->x = x;
		this->y = y;
	}

	/**
		Returns a pointer to the internal vector data.
		This allows you to pass the vector directly to a function expecting an array of 2 T
		Example:
		@code
			myvec2t<GLfloat> vector;
			glUniform2fv(someLocation, 1, vector);
		@endcode
		@return A pointer to the internal data, of type T
	*/
	operator float*()
	{
		return coord;
	}

	/**
		Performs component-wise vector addition of the vector and the given parameter vector, and returns the result.
		@param v Second operand of the vector addition
		Example:
		@code
			myvec2t<GLfloat> a;
			myvec2t<GLfloat> b;
			myvec2t<GLfloat> c = a + b;
		@endcode
		@return The result of the vector addition
	*/
	myvec2t<T> operator + (myvec2t<T> v) const
	{
		myvec2t<T> r;
		r.x = x + v.x;
		r.y = y + v.y;
		return r;
	}

	/**
		Performs component-wise vector subtraction of the vector and the given parameter vector, and returns the result.
		@param v Second operand of the vector subtraction
		Example:
		@code
			myvec2t<GLfloat> a;
			myvec2t<GLfloat> b;
			myvec2t<Glfloat> c = a - b;
		@endcode
		@return The result of the vector subtraction
	*/
	myvec2t<T> operator - (myvec2t<T> v) const
	{
		myvec2t<T> r;
		r.x = x - v.x;
		r.y = y - v.y;
		return r;
	}

	/**
		Multiplies all vector components with the given scalar parameter, and returns the result.
		@param v Scalar operand of the vector multiplication
		Example:
		@code
			myvec2t<GLfloat> a;
			GLfloat b;
			myvec2t<GLfloat> c = a * b;
		@endcode
		@return The result of the vector multiplication
	*/
	myvec2t<T> operator * (T v) const
	{
		myvec2t<T> r;
		r.x = x * v;
		r.y = y * v;
		return r;
	}

		/**
		Divides all vector components by the given scalar parameter, and returns the result.
		@param v Second operand (denominator) of the division
		Example:
		@code
			myvec2t<GLfloat> a;
			GLfloat b;
			myvec2t<GLfloat> c = a / b;
		@endcode
		@return The result of the vector division
	*/
	myvec2t<T> operator / (T v) const
	{
		myvec2t<T> r;
		r.x = x / v;
		r.y = y / v;
		return r;
	}

	/**
		Adds the given parameter vector to the vector, component-wise.
		The result of the addition is also returned, allowing for expressions like c = b += a.
		@param v The vector to add to this vector
		Example:
		@code
			myvec2t<GLfloat> a;
			myvec2t<GLfloat> b;
			b += a;
		@endcode
		@return The result of the vector addition
	*/
	myvec2t<T>& operator += (myvec2t<T> v)
	{
		x += v.x;
		y += v.y;
		return *this;
	}

	/**
		Subtracts the given parameter vector from this vector, component-wise.
		The result of the subtraction is also returned, allowing for expressions like c = b -= a.
		@param v The vector to subtract from this vector
		Example:
		@code
			myvec2t<GLfloat> a;
			myvec2t<GLfloat> b;
			b -= a;
		@endcode
		@return The result of the vector subtraction
	*/
	myvec2t<T>& operator -= (myvec2t<T> v)
	{
		x -= v.x;
		y -= v.y;
		return *this;
	}

		/**
		Multiplies all vector components with the given scalar parameter.
		The result of the multiplication is also returned, allowing for expressions like c = b *= a.
		@param v The vector to multiply with this vector
		Example:
		@code
			myvec2t<GLfloat> a;
			GLfloat b;
			b *= a;
		@endcode
		@return The result of the vector multiplication
	*/
	myvec2t<T>& operator *= (T v)
	{
		x *= v;
		y *= v;
		return *this;
	}

	/**
		Performs 2-component dot-product between the vector and the given parameter vector. The scalar result is returned.
		@param v The second operand of the dot-product operation
		Example:
		@code
			myvec2t<GLfloat> a;
			myvec2t<GLfloat> b;
			GLfloat c = a * b;
		@endcode
		@return The scalar result of the dot-product operation
	*/
	T operator * (myvec2t<T> v) const
	{
		return x*v.x + y*v.y;
	}

	/**
		Returns the squared length of the vector.
		Example:
		@code
			myvec2t<GLfloat> vec;
			float lengthSquared = vec.lengthSquared();
		@endcode
		@return The squared length of the vector
	*/
	T lengthSquared() const
	{
		return (*this) * (*this);
	}

	/**
		Returns the length of the vector.
		Example:
		@code
			myvec2t<GLfloat> vec;
			float length = vec.length();
		@endcode
		@return The length of the vector
	*/
	T length() const
	{
		return sqrt(lengthSquared());
	}

	/**
		Returns the vector normalized.
		Example:
		@code
			myvec2t<GLfloat> a;
			myvec2t<GLfloat> aNormalized = a.normalized();
		@endcode
		@return the normalized vector
	*/
	myvec2t normalized() const
	{
		float len = 1.0f / length();
		return *this * len;
	}
};

// Convenient defines
typedef myvec2t<float> myvec2;
typedef myvec2t<int> myvec2i;
typedef myvec2t<unsigned int> myvec2ui;
typedef myvec2t<short> myvec2s;
typedef myvec2t<unsigned short> myvec2us;
typedef myvec2t<char> myvec2b;
typedef myvec2t<unsigned char> myvec2ub;

template <typename T> class myvec4t;

//Forward declarations
template <typename T> class myvec3t
{
public:

	union
	{
		struct { T x, y, z; };
		T coord[3];
	};

	myvec3t()
	{
	}

	myvec3t(T x, T y, T z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	myvec3t(myvec4t<T> t)
	{
		this->x = t.x;
		this->y = t.y;
		this->z = t.z;
	}


	float dot( myvec3t<T> v )
	{
		return this->x*v.x + this->y*v.y + this->z*v.z;
	}

	operator float*()
	{
		return coord;
	}

	myvec3t<T> operator + (myvec3t<T> v) const 
	{
		myvec3t<T> r;
		r.x = x + v.x;
		r.y = y + v.y;
		r.z = z + v.z;
		return r;
	}

	myvec3t<T> operator - (myvec3t<T> v) const 
	{
		myvec3t<T> r;
		r.x = x - v.x;
		r.y = y - v.y;
		r.z = z - v.z;
		return r;
	}

	myvec3t<T> operator * (T v) const
	{
		myvec3t<T> r;
		r.x = x * v;
		r.y = y * v;
		r.z = z * v;
		return r;
	}

	myvec3t<T> operator / (T v) const
	{
		myvec3t<T> r;
		r.x = x / v;
		r.y = y / v;
		r.z = z / v;
		return r;
	}

	myvec3t<T>& operator += (myvec3t<T> v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	myvec3t<T>& operator -= (myvec3t<T> v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	myvec3t<T>& operator *= (T v)
	{
		x *= v;
		y *= v;
		z *= v;
		return *this;
	}

	myvec3t<T>& operator *= (const myvec3t<T> v) 
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;
		return *this;
	}

	T operator * (myvec3t<T> v) const
	{
		return x*v.x + y*v.y + z*v.z;
	}

	T lengthSquared() const
	{
		return (*this) * (*this);
	}

	T length() const
	{
		return sqrt(lengthSquared());
	}

	myvec3t normalized() const
	{
		float len = 1.0f / length();
		return *this * len;
	}

	myvec3t<T> crossProduct(myvec3t<T> v)
	{
		myvec3t<T> r;
		r.x = y*v.z - z*v.y;
		r.y = z*v.x - x*v.z;
		r.z = x*v.y - y*v.x;
		return r;
	}
};

typedef myvec3t<float> myvec3;
typedef myvec3t<int> myvec3i;
typedef myvec3t<unsigned int> myvec3ui;
typedef myvec3t<short> myvec3s;
typedef myvec3t<unsigned short> myvec3us;
typedef myvec3t<char> myvec3b;
typedef myvec3t<unsigned char> myvec3ub;

template <typename T>
	myvec3t<T> operator / (T v, myvec3t<T> const & rhs)
	{
		myvec3t<T> r;
		r.x = v/rhs.x;
		r.y = v/rhs.y;
		r.z = v/rhs.z;
		return r;
	}


template <typename T> class myvec4t
{
public:

	union
	{
		struct { T x, y, z, w; };
		T coord[4];
	};

	myvec4t()
	{
	}

	myvec4t( myvec3t<T> &t, T w )
	{
		this->x = t.x;
		this->y = t.y;
		this->z = t.z;
		this->w = w;
	}

	myvec4t(T x, T y, T z, T w)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}

	myvec4t(myvec3t<T>& v)
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
		this->w = 1.0f;
	}

	operator T*()
	{
		return coord;
	}
	
	myvec4t<T> operator + (myvec4t<T> v) const
	{
		myvec4t<T> r;
		r.x = x + v.x;
		r.y = y + v.y;
		r.z = z + v.z;
		r.w = w + v.w;
		return r;
	}

	myvec4t<T> operator - (myvec4t<T> v) const
	{
		myvec4t<T> r;
		r.x = x - v.x;
		r.y = y - v.y;
		r.z = z - v.z;
		r.w = w - v.w;
		return r;
	}

	myvec4t<T> operator * (T v) const
	{
		myvec4t<T> r;
		r.x = x * v;
		r.y = y * v;
		r.z = z * v;
		r.w = w * v;
		return r;
	}

	myvec4t<T> operator / (T v) const
	{
		myvec4t<T> r;
		r.x = x / v;
		r.y = y / v;
		r.z = z / v;
		r.w = w / v;
		return r;
	}

	myvec4t<T>& operator += (myvec4t<T> v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		w += v.w;
		return *this;
	}

	myvec4t<T>& operator -= (myvec4t<T> v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		w -= v.w;
		return *this;
	}

	myvec4t<T>& operator *= (T v)
	{
		x *= v;
		y *= v;
		z *= v;
		w *= v;
		return *this;
	}

	T operator * (myvec4t<T> v) const
	{
		return x*v.x + y*v.y + z*v.z + w*v.w;
	}

	T lengthSquared() const
	{
		return (*this) * (*this);
	}

	T length() const
	{
		return sqrt(lengthSquared());
	}

	myvec4t normalized() const
	{
		float len = 1.0f / length();
		return (*this * len);	
	}
};

typedef myvec4t<float> myvec4;
typedef myvec4t<int> myvec4i;
typedef myvec4t<unsigned int> myvec4ui;
typedef myvec4t<short> myvec4s;
typedef myvec4t<unsigned short> myvec4us;
typedef myvec4t<char> myvec4b;
typedef myvec4t<unsigned char> myvec4ub;


class mymat4
{
public:

	float m[16];

	operator float* ()
	{
		return m;
	}

	float& operator [] (int index)
	{
		return m[index];
	}

	myvec4 operator * (myvec4 v) const
	{
		myvec4 r;
		r.x = v.x*m[0] + v.y*m[4] + v.z*m[8] + v.w*m[12];
		r.y = v.x*m[1] + v.y*m[5] + v.z*m[9] + v.w*m[13];
		r.z = v.x*m[2] + v.y*m[6] + v.z*m[10] + v.w*m[14];
		r.w = v.x*m[3] + v.y*m[7] + v.z*m[11] + v.w*m[15];
		return r;
	}

	mymat4 operator * (mymat4 mat) const
	{
		mymat4 r;
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				r[i*4+j] = mat[i*4+0]*m[j] + mat[i*4+1]*m[4+j] + mat[i*4+2]*m[8+j] + mat[i*4+3]*m[12+j];
		return r;
	}

	mymat4& operator = (float* d)
	{
		for (int i = 0; i < 16; i++) m[i] = d[i];
		return *this;
	}

	mymat4& operator = (mymat4 mat)
	{
		for (int i = 0; i < 16; i++) m[i] = mat.m[i];
		return *this;
	}

	mymat4& operator *= (mymat4 mat)
	{
		*this = (*this) * mat;
		return *this;
	}

	myvec4 operator *= (myvec4 vec)
	{
		myvec4 vb = myvec4(0,0,0,0);
		for(int j = 0; j < 4; j++)
		{
			vb.x += vec[j]*m[4*j + 0];
			vb.y += vec[j]*m[4*j + 1];
			vb.z += vec[j]*m[4*j + 2];
			vb.w += vec[j]*m[4*j + 3];
		}
		return vb;
	}

	static mymat4 identity()
	{
		mymat4 r;
		r.m[0] = 1.0f;
		r.m[1] = 0.0f;
		r.m[2] = 0.0f;
		r.m[3] = 0.0f;
		r.m[4] = 0.0f;
		r.m[5] = 1.0f;
		r.m[6] = 0.0f;
		r.m[7] = 0.0f;
		r.m[8] = 0.0f;
		r.m[9] = 0.0f;
		r.m[10] = 1.0f;
		r.m[11] = 0.0f;
		r.m[12] = 0.0f;
		r.m[13] = 0.0f;
		r.m[14] = 0.0f;
		r.m[15] = 1.0f;
		return r;
	}

	static mymat4 rotation(float angleDeg, myvec3 axis)
	{	
		float angle = (float)(angleDeg * M_PI)  / 180.0f;
		axis = axis.normalized();
		float s = sinf(angle);
		float c = cosf(angle);
		mymat4 r = mymat4::identity();
		r[0] = c + (1-c)*axis.x*axis.x;
		r[4] = (1-c)*axis.x*axis.y - s*axis.z;
		r[8] = (1-c)*axis.x*axis.z + s*axis.y;

		r[1] = (1-c)*axis.y*axis.x + s*axis.z;
		r[5] = c + (1-c)*axis.y*axis.y;
		r[9] = (1-c)*axis.y*axis.z - s*axis.x;
		
		r[2] = (1-c)*axis.z*axis.x - s*axis.y;
		r[6] = (1-c)*axis.z*axis.y + s*axis.x;
		r[10] = c + (1-c)*axis.z*axis.z;
		return r;
	}

	static mymat4 translation(float x, float y, float z)
	{
		mymat4 r = mymat4::identity();
		r[12] = x;
		r[13] = y;
		r[14] = z;
		return r;
	}

	static mymat4 translation(myvec3 const &t)
	{
		mymat4 r = mymat4::identity();
		r[12] = t.x;
		r[13] = t.y;
		r[14] = t.z;
		return r;
	}

	mymat4 transposed() const
	{
		mymat4 ret;
		for (int y = 0; y < 4; ++y)
		{
			for (int x = 0; x < 4; ++x)
			{
				ret[x + y*4] = m[y + x*4];
			}
		}

		return ret;
	}

	static mymat4 scale(myvec3 v)
	{
		mymat4 m = mymat4::identity();
		m[0] = v.x;
		m[5] = v.y;
		m[10] = v.z;
		return m;
	}

	static mymat4 scale(float x, float y, float z)
	{
		myvec3 s(x, y, z);
		return scale(s);
	}

	static mymat4 lookAt(myvec3 camera, myvec3 target, myvec3 up)
	{
#if 0
		myvec3 zaxis = (camera - target).normalized();    // The "forward" vector.
		myvec3 xaxis = ((up).crossProduct(zaxis)).normalized();// The "right" vector.
		myvec3 yaxis = (zaxis).crossProduct(xaxis);     // The "up" vector.
 
    // Create a 4x4 view matrix from the right, up, forward and eye position vectors
		mymat4 viewMatrix = {
	              xaxis.x,            yaxis.x,            zaxis.x,       0 ,
			      xaxis.y,            yaxis.y,            zaxis.y,       0 ,
			      xaxis.z,            yaxis.z,            zaxis.z,       0 ,
			-(xaxis).dot(target), -(yaxis).dot(target), -( zaxis).dot(target),  1 
		};
		return viewMatrix;
#else
		myvec3 forward = (target - camera).normalized();

		myvec3 side = forward.crossProduct(up);
		up = side.crossProduct(forward);
		up = up.normalized();
		side = side.normalized();

		mymat4 m = mymat4::identity();

		m[0] = side.x;
		m[4] = side.y;
		m[8] = side.z;

		m[1] = up.x;
		m[5] = up.y;
		m[9] = up.z;

		m[2] = -forward.x;
		m[6] = -forward.y;
		m[10] = -forward.z;

		mymat4 t = mymat4::translation(-camera.x, -camera.y, -camera.z);

		return m * t;
#endif
	}

	static mymat4 orthographic( float width, float height )
	{
		mymat4 p;
		p.m[0] = 2.f/width;
		p.m[1] = 0;
		p.m[2] = 0;
		p.m[3] = 0;
		p.m[4] = 0;
		p.m[5] = 2.f/(height);
		p.m[6] = 0;
		p.m[7] = 0;
		p.m[8] = 0;
		p.m[9] = 0;
		p.m[10] = -1;
		p.m[11] = 0;
		p.m[12] = 0;
		p.m[13] = 0;
		p.m[14] = 0;
		p.m[15] = 1;
		return p;
	}

	static mymat4 perspective(float fovDeg, float aspect, float nearz, float farz)
	{
		float sine, cotangent;
		float radians = (float)fovDeg / 2.0f * M_PI / 180.0f;

		float zdelta = farz - nearz;
		sine = sinf(radians);

		cotangent = cosf(radians) / sine;

		mymat4 m = mymat4::identity();
		m[0] = cotangent / aspect;
		m[5] = cotangent;
		m[10] = -(farz + nearz) / zdelta;
		m[11] = -1;
		m[14] = -2 * nearz * farz / zdelta;
		m[15] = 0;

		return m;
	}

	bool invert(mymat4& out) const
	{
		/* http://www.gamedev.net/topic/648190-algorithm-for-4x4-matrix-inverse/ */
		double det;
		int i;

		out.m[0] = m[5]  * m[10] * m[15] - 
				 m[5]  * m[11] * m[14] - 
				 m[9]  * m[6]  * m[15] + 
				 m[9]  * m[7]  * m[14] +
				 m[13] * m[6]  * m[11] - 
				 m[13] * m[7]  * m[10];

		out.m[4] = -m[4]  * m[10] * m[15] + 
				  m[4]  * m[11] * m[14] + 
				  m[8]  * m[6]  * m[15] - 
				  m[8]  * m[7]  * m[14] - 
				  m[12] * m[6]  * m[11] + 
				  m[12] * m[7]  * m[10];

		out.m[8] = m[4]  * m[9] * m[15] - 
				 m[4]  * m[11] * m[13] - 
				 m[8]  * m[5] * m[15] + 
				 m[8]  * m[7] * m[13] + 
				 m[12] * m[5] * m[11] - 
				 m[12] * m[7] * m[9];

		out.m[12] = -m[4]  * m[9] * m[14] + 
				   m[4]  * m[10] * m[13] +
				   m[8]  * m[5] * m[14] - 
				   m[8]  * m[6] * m[13] - 
				   m[12] * m[5] * m[10] + 
				   m[12] * m[6] * m[9];

		out.m[1] = -m[1]  * m[10] * m[15] + 
				  m[1]  * m[11] * m[14] + 
				  m[9]  * m[2] * m[15] - 
				  m[9]  * m[3] * m[14] - 
				  m[13] * m[2] * m[11] + 
				  m[13] * m[3] * m[10];

		out.m[5] = m[0]  * m[10] * m[15] - 
				 m[0]  * m[11] * m[14] - 
				 m[8]  * m[2] * m[15] + 
				 m[8]  * m[3] * m[14] + 
				 m[12] * m[2] * m[11] - 
				 m[12] * m[3] * m[10];

		out.m[9] = -m[0]  * m[9] * m[15] + 
				  m[0]  * m[11] * m[13] + 
				  m[8]  * m[1] * m[15] - 
				  m[8]  * m[3] * m[13] - 
				  m[12] * m[1] * m[11] + 
				  m[12] * m[3] * m[9];

		out.m[13] = m[0]  * m[9] * m[14] - 
				  m[0]  * m[10] * m[13] - 
				  m[8]  * m[1] * m[14] + 
				  m[8]  * m[2] * m[13] + 
				  m[12] * m[1] * m[10] - 
				  m[12] * m[2] * m[9];

		out.m[2] = m[1]  * m[6] * m[15] - 
				 m[1]  * m[7] * m[14] - 
				 m[5]  * m[2] * m[15] + 
				 m[5]  * m[3] * m[14] + 
				 m[13] * m[2] * m[7] - 
				 m[13] * m[3] * m[6];

		out.m[6] = -m[0]  * m[6] * m[15] + 
				  m[0]  * m[7] * m[14] + 
				  m[4]  * m[2] * m[15] - 
				  m[4]  * m[3] * m[14] - 
				  m[12] * m[2] * m[7] + 
				  m[12] * m[3] * m[6];

		out.m[10] = m[0]  * m[5] * m[15] - 
				  m[0]  * m[7] * m[13] - 
				  m[4]  * m[1] * m[15] + 
				  m[4]  * m[3] * m[13] + 
				  m[12] * m[1] * m[7] - 
				  m[12] * m[3] * m[5];

		out.m[14] = -m[0]  * m[5] * m[14] + 
				   m[0]  * m[6] * m[13] + 
				   m[4]  * m[1] * m[14] - 
				   m[4]  * m[2] * m[13] - 
				   m[12] * m[1] * m[6] + 
				   m[12] * m[2] * m[5];

		out.m[3] = -m[1] * m[6] * m[11] + 
				  m[1] * m[7] * m[10] + 
				  m[5] * m[2] * m[11] - 
				  m[5] * m[3] * m[10] - 
				  m[9] * m[2] * m[7] + 
				  m[9] * m[3] * m[6];

		out.m[7] = m[0] * m[6] * m[11] - 
				 m[0] * m[7] * m[10] - 
				 m[4] * m[2] * m[11] + 
				 m[4] * m[3] * m[10] + 
				 m[8] * m[2] * m[7] - 
				 m[8] * m[3] * m[6];

		out.m[11] = -m[0] * m[5] * m[11] + 
				   m[0] * m[7] * m[9] + 
				   m[4] * m[1] * m[11] - 
				   m[4] * m[3] * m[9] - 
				   m[8] * m[1] * m[7] + 
				   m[8] * m[3] * m[5];

		out.m[15] = m[0] * m[5] * m[10] - 
				  m[0] * m[6] * m[9] - 
				  m[4] * m[1] * m[10] + 
				  m[4] * m[2] * m[9] + 
				  m[8] * m[1] * m[6] - 
				  m[8] * m[2] * m[5];

		det = m[0] * out.m[0] + m[1] * out.m[4] + m[2] * out.m[8] + m[3] * out.m[12];

		if (det == 0)
			return false;

		det = 1.0 / det;

		for (i = 0; i < 16; i++)
			out.m[i] = out.m[i] * det;

		return true;
	}
};

#endif /* _MATH_HELPERS_H_ */
