
/*****************************************************************************/
template <class T, XUINT N>
class Vector
{
protected:
	T vector[N];

public:
	Vector()
	{
		CLEAR_CONSTRUCTOR();
	}

	~Vector(){};


	//Set/Get functions
	inline void set(XUINT i, T data);
	inline void set(Vector<T, N> *v);
    inline void set(Vector<T, N> &v);
	inline T get(XUINT i);
	inline T *get(void);
	inline void change(XUINT i, T c);
	inline void clear(void);

	//Operators
	inline T &operator[](XUINT i);
	inline Vector<T, N> operator+(Vector<T, N> &right);
	inline void operator+=(Vector<T, N> &right);
	inline Vector<T, N> operator-(Vector<T, N> &right);
	inline void operator-=(Vector<T, N> &right);
	inline bool operator==(Vector<T, N> &right);
	inline bool operator!=(Vector<T, N> &right);
	inline bool operator<=(Vector<T, N> &right);
	inline bool operator>=(Vector<T, N> &right);

	//Vector functions
	inline T length(void);
	inline T normalize(void);
	inline T normalize(Vector<T, N> &v);
	inline void scale(T s);
	inline void scale(XUINT i, T s);
	inline void scale(Vector<T, N> &v, T s);
	inline T dotProduct(Vector<T, N> &v);
	inline void crossProduct(Vector<T, N> &u, Vector<T, N> &v);
	inline void crossProduct(Vector<T, N> &v);
	inline void translate(Vector<T, N> &v);
	inline bool isUnitLength(void);
};

typedef Vector<XFLOAT, 2> Vector2D;
typedef Vector<XFLOAT, 3> Vector3D;
typedef Vector<XFLOAT, 4> Vector4D;

/*****************************************************************************/
template <class T, XUINT N>
inline void Vector<T, N>::set(XUINT i, T data)
{
	if ((i >= 0) && (i < N))
	{
		vector[i] = data;
	}
}
/*****************************************************************************/
template <class T, XUINT N>
inline void Vector<T, N>::set(Vector<T, N> *v)
{
	if (v)
	{
		memcpy(vector, v->get(), XSIZE(T) * N);
	}
}

/*****************************************************************************/
template <class T, XUINT N>
inline void Vector<T, N>::set(Vector<T, N> &v)
{
    memcpy(vector, v.get(), XSIZE(T) * N);
}
/*****************************************************************************/
template <class T, XUINT N>
inline T Vector<T, N>::get(XUINT i)
{
	T rt = 0;
	
	if ((i >= 0) && (i < N))
	{
		rt = vector[i];
	}

	return(rt);
}

/*****************************************************************************/
template <class T, XUINT N>
inline T *Vector<T, N>::get(void)
{
	return(vector);
}
/*****************************************************************************/
template <class T, XUINT N>
inline void Vector<T, N>::change(XUINT i, T c)
{
	if ((i >= 0) && (i < N))
	{
		vector[i] += c;
	}
}
/*****************************************************************************/
template <class T, XUINT N>
inline void Vector<T, N>::clear(void)
{
	memset(vector, 0, XSIZE(T) * N);
}
/*****************************************************************************/
//Operators
template <class T, XUINT N>
inline T &Vector<T, N>::operator[](XUINT i)
{
	return(vector[i]);
}
/*****************************************************************************/
template <class T, XUINT N>
inline Vector<T, N> Vector<T, N>::operator+(Vector<T, N> &right)
{
	Vector<T, N> rt;

	for (int j = 0; j < N; j++)
	{
		rt[j] = vector[j] + right[j];
	}

	return(rt);
}
/*****************************************************************************/
template <class T, XUINT N>
inline void Vector<T, N>::operator+=(Vector<T, N> &right)
{
	for (int j = 0; j < N; j++)
	{
		vector[j] += right[j];
	}
}
/*****************************************************************************/
template <class T, XUINT N>
inline Vector<T, N> Vector<T, N>::operator-(Vector<T, N> &right)
{
	Vector<T, N> rt;
	for (int j = 0; j < N; j++)
	{
		rt[j] = vector[j] - right[j];
	}
	
	return(rt);
}
/*****************************************************************************/
template <class T, XUINT N>
inline void Vector<T, N>::operator-=(Vector<T, N> &right)
{
	for (int j = 0; j < N; j++)
	{
		vector[j] += right[j];
	}
}
/*****************************************************************************/
template <class T, XUINT N>
inline bool Vector<T, N>::operator==(Vector<T, N> &right)
{
	bool rt = true;

	for (int j = 0; j < N; j++)
	{
		if (vector[j] != right[j])
		{
			rt = false;
			break;
		}
	}

	return(rt);
}
/*****************************************************************************/
template <class T, XUINT N>
inline bool Vector<T, N>::operator!=(Vector<T, N> &right)
{
	bool rt = true;

	for (int j = 0; j < N; j++)
	{
		if (vector[j] == right[j])
		{
			rt = false;
			break;
		}
	}

	return(rt);
}
/*****************************************************************************/
//Vector functions
template <class T, XUINT N>
inline T Vector<T, N>::length(void)
{
	T len, total = 0;

	for (int j = 0; j < N; j++)
	{
		total += vector[j] * vector[j];
	}

	len = (T)sqrt(total);

	return(len);
}
/*****************************************************************************/
template <class T, XUINT N>
inline T Vector<T, N>::normalize(void)
{
	T len;

	len = length();

	for (int j = 0; j < N; j++)
	{
		vector[j] /= len;
	}

	return(len);
}
/*****************************************************************************/
template <class T, XUINT N>
inline T Vector<T, N>::normalize(Vector<T, N> &v)
{
	T len; 

	len = v.length();

	for (int j = 0; j < N; j++)
	{
		vector[j] = v[j] / len;
	}

	return(len);
}
/*****************************************************************************/
template <class T, XUINT N>
inline void Vector<T, N>::scale(T s)
{
	for (int j = 0; j < N; j++)
	{
		vector[j] *= s;
	}
}
/*****************************************************************************/
template <class T, XUINT N>
inline void Vector<T, N>::scale(XUINT i, T s)
{
	if ((i >= 0) && (i < N))
	{
		vector[i] *= s;
	}
}
/*****************************************************************************/
template <class T, XUINT N>
inline void Vector<T, N>::scale(Vector<T, N> &v, T s)
{
	for (int j = 0; j < N; j++)
	{
		vector[j] = v[j] * s;
	}
}
/*****************************************************************************/
template <class T, XUINT N>
inline T Vector<T, N>::dotProduct(Vector<T, N> &v)
{
	T dotP = 0;

	for (int j = 0; j < N; j++)
	{
		dotP += vector[j] * v[j];
	}

	return(dotP);
}
/*****************************************************************************/
template <class T, XUINT N>
inline void Vector<T, N>::crossProduct(Vector<T, N> &u, Vector<T, N> &v)
{
	if (N == 3)
	{
		vector[0] = (u[1]*v[2]) - (u[2]*v[1]);
		vector[1] = (u[2]*v[0]) - (u[0]*v[2]);
		vector[2] = (u[0]*v[1]) - (u[1]*v[0]);
	}
}
/*****************************************************************************/
template <class T, XUINT N>
inline void Vector<T, N>::crossProduct(Vector<T, N> &v)
{
	if (N == 3)
	{
		T x=vector[0], y=vector[1], z=vector[2];

		vector[0] = (y*v[2]) - (z*v[1]);
		vector[1] = (z*v[0]) - (x*v[2]);
		vector[2] = (x*v[1]) - (y*v[0]);
	}
}
		
/*****************************************************************************/
template <class T, XUINT N>
inline void Vector<T, N>::translate(Vector<T, N> &v)
{
	for (int j = 0; j < N; j++)
	{
		vector[j] += v[j];
	}
}
/*****************************************************************************/
template <class T, XUINT N>
inline bool Vector<T, N>::operator<=(Vector<T, N> &right)
{
	bool rt = true;
	DynamicArray<XBYTE> flags;

	for (XUINT j = 0; j < N; j++)
	{
		if (vector[j] > right[j])
		{
			rt = false;
			break;
		}
	}

	return(rt);
}
/*****************************************************************************/
template<class T, XUINT N>
inline bool Vector<T, N>::operator>=(Vector<T, N> &right)
{
	bool rt = true;

	for (XUINT j = 0; j < N; j++)
	{
		if (vector[j] < right[j])
		{
			rt = false;
			break;
		}
	}

	return(rt);
}
/*****************************************************************************/
template<class T, XUINT N>
inline bool Vector<T, N>::isUnitLength(void)
{
	T total = vector[0] * vector[0];

	for (XUINT j = 1; j < N; j++)
	{
		total += (vector[j] * vector[j]);
	}

	if (total == (T)1.00)
	{
		return(true);
	}

	return(false);
}
