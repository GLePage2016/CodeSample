
/*****************************************************************************/
template <class T, XUINT X>
class Matrix
{
protected:
	//Matrix data
	Vector<T, X> rows[X];

public:
	//Constructor/destructor	
	Matrix()
	{
		CLEAR_CONSTRUCTOR();
	}

	~Matrix(){};

	//Set/Get functions
	inline void setIdentity(void);
	inline void setZero(void);
	inline void setRow(XUINT r, Vector<T, X> &row);
	inline void setColumn(XUINT c, Vector<T, X> &column);
	inline Vector<T, X> &getRow(XUINT row);
	inline Vector<T, X> getColumn(XUINT column);

	//**Operator functions**//

	//Assignment operators
	inline Vector<T, X> &operator[](XUINT row);
	inline Matrix<T, X> &operator=(Matrix &right);

	//Logical operators
	inline bool operator==(Matrix<T, X> &right);
	inline bool operator!=(Matrix<T, X> &right);

	//Mathematical operators
	inline Matrix<T, X> operator+(Matrix<T, X> &right);
	inline void operator+=(Matrix<T, X> &right);
	inline Matrix<T, X> operator-(Matrix<T, X> &right);
	inline void operator-=(Matrix<T, X> &right);
	inline Matrix<T, X> operator*(Matrix<T, X> &right);

	//**Mathematical functions**//

	//Vector transformation functions
	inline void transformVector(Vector<T, X> &v);
	inline void transformVector(Vector<T, X> *v);

	//Scalar functions
	inline void scale(T s);
	inline void scale(Matrix<T, X> &m, T s);

	//Matrix functions
	inline void transpose(void);
	inline void transpose(Matrix<T, X> &m);

	//Inverse functions will only work on 3x3 matrices
	inline bool inverse(XFLOAT tolerance = 1e-06);
	inline bool inverse(Matrix<T, X> &m, XFLOAT tolerance = 1e-06);

	//Determinant functions
	inline XFLOAT determinant22(void);
	inline XFLOAT determinant33(void);
	inline XFLOAT determinant44(void);

	//Translation matrix
	inline void createTranslation(Vector<T, X> &v);

	//Conversion from quaternion
	inline void createFromQuaternion(Vector<T, 4> &quat);
};

typedef Matrix<XFLOAT, 3> Matrix33;
typedef Matrix<XFLOAT, 4> Matrix44;
/*****************************************************************************/
//Set/Get functions
template <class T, XUINT X>
inline void Matrix<T, X>::setIdentity(void)
{
	XUINT t = 0;
	
	for (XUINT j = 0; j < X; j++)
	{
		rows[j].clear();
		rows[j][t] = 1;
		t++;
	}
}
/*****************************************************************************/
template <class T, XUINT X>
inline void Matrix<T, X>::setZero(void)
{
	for (XUINT j = 0; j < X; j++)
	{	
		rows[j].clear();
	}
}
/*****************************************************************************/
template <class T, XUINT X>
inline void Matrix<T, X>::setRow(XUINT r, Vector<T, X> &row)
{
	if ((r >= 0) && (r < X))
	{
		rows[r] = row;
	}
}
/*****************************************************************************/
template <class T, XUINT X>
inline void Matrix<T, X>::setColumn(XUINT c, Vector<T, X> &column)
{
	if ((c >= 0) && (c < X))
	{
		for (XUINT j = 0; j < X; j++)
		{
			rows[j][c] = column[j];
		}
	}
}
/*****************************************************************************/
template <class T, XUINT X>
inline Vector<T, X> &Matrix<T, X>::getRow(XUINT row)
{
	if ((row >= 0) && (row < X))
	{
		return(rows[row]);
	}

	return(rows[0]);
}
/*****************************************************************************/
template <class T, XUINT X>
inline Vector<T, X> Matrix<T, X>::getColumn(XUINT column)
{
	Vector<T, X> rt;

	if ((column >= 0) && (column < X))
	{
		for (XUINT j = 0; j < X; j++)
		{
			rt[j] = rows[j][X];
		}
	}

	return(rt);
}

/*****************************************************************************/
//**Operator functions**//
/*****************************************************************************/
//Assignment operators
template <class T, XUINT X>
inline Vector<T, X> &Matrix<T, X>::operator[](XUINT row)
{
	return(rows[row]);
}
/*****************************************************************************/
template <class T, XUINT X>
inline Matrix<T, X> &Matrix<T, X>::operator=(Matrix<T, X> &right)
{
	for (XUINT j = 0; j < X; j++)
	{
		memcpy(&rows[j], right[j], sizeof(Vector<T, X>));
	}

	return((Matrix<T, X> &)*this);
}
/*****************************************************************************/
//Logical operators
template <class T, XUINT X>
inline bool Matrix<T, X>::operator==(Matrix<T, X> &right)
{
	bool rt = true;

	for (XUINT j = 0; j < X; j++)
	{
		if (rows[j] != right[j])
		{
			rt = false;
			break;
		}
	}

	return(rt);
}
/*****************************************************************************/
template <class T, XUINT X>
inline bool Matrix<T, X>::operator!=(Matrix<T, X> &right)
{
	bool rt = true;

	for (XUINT j = 0; j < X; j++)
	{
		if (rows[j] == right[j])
		{
			rt = false;
			break;
		}
	}

	return(rt);
}
/*****************************************************************************/
//Mathematical operators
template <class T, XUINT X>
inline Matrix<T, X> Matrix<T, X>::operator+(Matrix<T, X> &right)
{
	Matrix<T, X> rt;

	for (XUINT j = 0; j < X; j++)
	{
		rt[j] = rows[j] + right[j];
	}

	return(rt);
}
/*****************************************************************************/
template <class T, XUINT X>
inline void Matrix<T, X>::operator+=(Matrix<T, X> &right)
{
	for (XUINT j = 0; j < X; j++)
	{
		rows[j] += right[j];
	}
}
/*****************************************************************************/
template <class T, XUINT X>
inline Matrix<T, X> Matrix<T, X>::operator-(Matrix<T, X> &right)
{
	Matrix<T, X> rt;

	for (XUINT j = 0; j < Y; j++)
	{
		rt[j] = rows[j] - right[j];
	}

	return(rt);
}
/*****************************************************************************/
template <class T, XUINT X>
inline void Matrix<T, X>::operator-=(Matrix<T, X> &right)
{
	for (XUINT j = 0; j < X; j++)
	{
		rows[j] -= right[j];
	}
}
/*****************************************************************************/
//Assumes square matrices
template <class T, XUINT X>
inline Matrix<T, X> Matrix<T, X>::operator*(Matrix<T, X> &right)
{
	Matrix<T, X> rt;
	
	for (XUINT row = 0; row < X; row++)
	{
		for (XUINT col = 0; col < X; col++)
		{
			for (XUINT j = 0; j < X; j++)
			{
				rt[row][col] = rows[row][j] * right[j][col];
			}
		}
	}

	return(rt);
}
/*****************************************************************************/
//**Mathematical functions**//
/*****************************************************************************/
//Vector transformation functions
template <class T, XUINT X>
inline void Matrix<T, X>::transformVector(Vector<T, X> &v)
{
	Vector<T, X> rt;

	for (XUINT row = 0; row < X; row++)
	{
		for (XUINT col = 0; col < X; col++)
		{
			rt[row] += rows[row][col] * v[col];
		}
	}

	memcpy(&v, &rt, sizeof(Vector<T, X>));
}
/*****************************************************************************/
//Vector transformation functions
template <class T, XUINT X>
inline void Matrix<T, X>::transformVector(Vector<T, X> *v)
{
	Vector<T, X> rt;

	for (XUINT row = 0; row < X; row++)
	{
		for (XUINT col = 0; col < X; col++)
		{
			rt[row] += rows[row][col] * v->get(col);
		}
	}

	memcpy(&v, &rt, sizeof(Vector<T, X>));
}
/*****************************************************************************/
//Scalar functions
template <class T, XUINT X>
void Matrix<T, X>::scale(T s)
{
	for (XUINT row = 0; row < X; row++)
	{
		rows[row].scale(s);
	}
}
/*****************************************************************************/
template <class T, XUINT X>
void Matrix<T, X>::scale(Matrix<T, X> &m, T s)
{
	for (XUINT row = 0; row < X; row++)
	{
		rows[row].scale(m[row], s);
	}
}
/*****************************************************************************/
//Matrix functions
template <class T, XUINT X>
inline void Matrix<T, X>::transpose(void)
{
	Matrix<T, X> temp;

	for (XUINT row = 0; row < X; row++)
	{
		for (XUINT col = 0; col < X; col++)
		{
			temp[row][col] = rows[col][row];
		}
	}

	memcpy(this, temp, sizeof(Matrix<T, X>));
}
/*****************************************************************************/
template <class T, XUINT X>
inline void Matrix<T, X>::transpose(Matrix<T, X> &m)
{
	for (XUINT row = 0; row < X; row++)
	{
		for (XUINT col = 0; col < X; col++)
		{
			rows[row][col] = m[col][row];
		}
	}
}
/*****************************************************************************/
//NOTE:
//Will only work on a 3x3 matrix
template <class T, XUINT X>
inline bool Matrix<T, X>::inverse(XFLOAT tolerance)
{
	if (X == 3)
	{
		T r00, r01, r02;
		T r10, r11, r12;
		T r20, r21, r22;
		Matrix<T, X> temp;

		r00 = rows[0][0];
		r01 = rows[0][1];
		r02 = rows[0][2];
		r10 = rows[1][0];
		r11 = rows[1][1];
		r12 = rows[1][2];
		r20 = rows[2][0];
		r21 = rows[2][1];
		r22 = rows[2][2];

		temp[0][0] = (r11 * r22) - (r12 * r21);
		temp[0][1] = (r02 * r21) - (r01 * r22);
		temp[0][2] = (r01 * r12) - (r02 * r11);

		temp[1][0] = (r12 * r20) - (r10 * r22);
		temp[1][1] = (r00 * r22) - (r02 * r20);
		temp[1][2] = (r02 * r10) - (r00 * r12);

		temp[2][0] = (r10 * r21) - (r11 * r20);
		temp[2][1] = (r01 * r20) - (r00 * r21);
		temp[2][2] = (r00 * r11) - (r01 * r10);

		T fDet = (r00 * temp[0][0]) + 
			     (r01 * temp[1][0]) +
				 (r02 * temp[2][0]);

		if (ABS(fDet) <= fTolerance)
		{
			return(false);
		}
		else
		{
			fInvDet = 1.0 / fDet;
			for (XUINT row = 0; row < X; row++)
			{
				for (XUINT col = 0; col < X; col++)
				{
					temp[row][col] *= fInvDet;
				}
			}

			memcpy(this, temp, sizeof(Matrix<T, X>));
		}
	}
	else
	{
		return(false);
	}

	return(true);
}
/*****************************************************************************/
template <class T, XUINT X>
inline bool Matrix<T, X>::inverse(Matrix<T, X> &m, XFLOAT tolerance)
{
	if (X == 3)
	{
		T r00, r01, r02;
		T r10, r11, r12;
		T r20, r21, r22;
		Matrix<T, X> temp;

		r00 = m[0][0];
		r01 = m[0][1];
		r02 = m[0][2];
		r10 = m[1][0];
		r11 = m[1][1];
		r12 = m[1][2];
		r20 = m[2][0];
		r21 = m[2][1];
		r22 = m[2][2];

		temp[0][0] = (r11 * r22) - (r12 * r21);
		temp[0][1] = (r02 * r21) - (r01 * r22);
		temp[0][2] = (r01 * r12) - (r02 * r11);

		temp[1][0] = (r12 * r20) - (r10 * r22);
		temp[1][1] = (r00 * r22) - (r02 * r20);
		temp[1][2] = (r02 * r10) - (r00 * r12);

		temp[2][0] = (r10 * r21) - (r11 * r20);
		temp[2][1] = (r01 * r20) - (r00 * r21);
		temp[2][2] = (r00 * r11) - (r01 * r10);

		T fDet = (r00 * temp[0][0]) + 
			     (r01 * temp[1][0]) +
				 (r02 * temp[2][0]);

		if (ABS(fDet) <= fTolerance)
		{
			return(false);
		}
		else
		{
			fInvDet = 1.0 / fDet;
			for (XUINT row = 0; row < X; row++)
			{
				for (XUINT col = 0; col < X; col++)
				{
					temp[row][col] *= fInvDet;
				}
			}

			memcpy(this, temp, sizeof(Matrix<T, X>));
		}
	}
	else
	{
		return(false);
	}

	return(true);
}
/*****************************************************************************/
template <class T, XUINT X>
inline XFLOAT Matrix<T, X>::determinant22(void)
{
	if (X == 2)
	{
		return((rows[0][0] * rows[1][1]) - (rows[0][1] * rows[1][0]));
	}

	return(1e-06);
}
/*****************************************************************************/
template <class T, XUINT X>
inline XFLOAT Matrix<T, X>::determinant33(void)
{
	if (X == 3)
	{
		XFLOAT fDet;
		XFLOAT fCoFactor00, fCoFactor10, fCoFactor20;

		fCoFactor00 = (rows[1][1] * rows[2][2]) - 
			          (rows[1][2] * rows[2][1]);
		fCoFactor10 = (rows[1][2] * rows[2][0]) - 
					  (rows[1][0] * rows[2][2]);
		fCoFactor20 = (rows[1][0] * rows[2][1]) -
					  (rows[1][1] * rows[2][0]);

		fDet = (rows[0][0] * fCoFactor00) + 
			   (rows[1][1] * fCoFactor10) +
			   (rows[2][2] * fCoFactor20);

		return(fDet);
	}


	return(1e-06);
}
/*****************************************************************************/
template <class T, XUINT X>
inline XFLOAT Matrix<T, X>::determinant44(void)
{
	if (X == 4)
	{
		XFLOAT e, f, g, h;
		XFLOAT i, j, k, l;
		XFLOAT m, n, o, p;
		XFLOAT det, detsub1, detsub2, detsub3, detsub4;

		//Cache row two
		e = rows[1][0];
		f = rows[1][1];
		g = rows[1][2];
		h = rows[1][3];

		//Cache row three
		i = rows[2][0];
		j = rows[2][1];
		k = rows[2][2];
		l = rows[2][3];

		//Cache row four
		m = rows[3][0];
		n = rows[3][1];
		o = rows[3][2];
		p = rows[3][3];

		//Calculate det sub 1
		detsub1 = (f * ((k*p)-(l*o))) + 
				  (g * ((j*p)-(n*l))) + 
				  (h * ((j*o)-(n*k)));

		//Calculate det sub 2
		detsub2 = (e * ((k*p)-(o*l))) +
				  (g * ((i*p)-(m*l))) +
				  (h * ((i*o)-(m*k)));

		//Calculate det sub 3
		detsub3 = (e * ((j*p)-(n*l))) +
				  (f * ((i*p)-(m*l))) +
				  (h * ((i*n)-(m*j)));

		//Calculate det sub 4
		detsub4 = (e * ((j*o)-(n*k))) +
				  (f * ((i*o)-(m*k))) +
				  (g * ((i*n)-(m*j)));

		//Calculate det
		det = (a * detsub1) - (b * detsub2) + 
			  (c * detsub3) - (d * detsub4);

		return(det);
	}

	return(1e-06);
}

/*****************************************************************************/
template <class T, XUINT X>
inline void Matrix<T, X>::createFromQuaternion(Vector<T, 4> &quat)
{
	T X, Y, Z, W;
	T xx, xy, xz, xw;
	T yy, yz, yw;
	T zz, zw;
	
	X = quat[0];
	Y = quat[1];
	Z = quat[2];
	W = quat[3];

	xx = X * X;
	xy = X * Y;
	xz = X * Z;
	xw = X * W;

	yy = Y * Y;
	yz = Y * Z;
	yw = Y * W;

	zz = Z * Z;
	zw = Z * W;

	rows[0][0] = 1 - 2 * (yy + zz);
	rows[0][1] =     2 * (xy + zw);
	rows[0][2] =     2 * (xz - yw);

	rows[1][0] =     2 * (xy - zw);
	rows[1][1] = 1 - 2 * (xx + zz);
	rows[1][2] =     2 * (yz + xw);

	rows[2][0] =     2 * (xz + yw);
	rows[2][1] =     2 * (yz - xw);
	rows[2][2] = 1 - 2 * (xx + yy);

	if (X == 4)
	{
		rows[0][3] = rows[1][3] = rows[2][3] = 0;
		rows[3].clear();
		rows[3][3] = 1;
	}
}


