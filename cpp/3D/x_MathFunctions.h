
class MathFunctions
{
public:
	inline XFLOAT degreesToRadians(XFLOAT &degree);
    inline XFLOAT degreesToRadiansN(XFLOAT degree);
	inline XFLOAT radiansToDegrees(XFLOAT &radian);
    inline XFLOAT radiansToDegreesN(XFLOAT radian);
	inline Matrix33 createXRotationMatrixRadians(XFLOAT &radian);
	inline Matrix33 createXRotationMatrixDegrees(XFLOAT &degree);
	inline Matrix33 createYRotationMatrixRadians(XFLOAT &radian);
	inline Matrix33 createYRotationMatrixDegrees(XFLOAT &degree);
	inline Matrix33 createZRotationMatrixRadians(XFLOAT &radian);
	inline Matrix33 createZRotationMatrixDegrees(XFLOAT &degree);
	inline Matrix33 createXRotationMatrixRadiansN(XFLOAT radian);
	inline Matrix33 createXRotationMatrixDegreesN(XFLOAT degree);
	inline Matrix33 createYRotationMatrixRadiansN(XFLOAT radian);
	inline Matrix33 createYRotationMatrixDegreesN(XFLOAT degree);
	inline Matrix33 createZRotationMatrixRadiansN(XFLOAT radian);
	inline Matrix33 createZRotationMatrixDegreesN(XFLOAT degree);
	inline Matrix33 createXYZRotationMatrixRadians(
		XFLOAT &radianX, XFLOAT &radianY, XFLOAT &radianZ);
	inline Matrix33 createXYZRotationMatrixDegrees(
		XFLOAT &degreeX, XFLOAT &degreeY, XFLOAT &degreeZ);
	inline Matrix33 createXYZRotationMatrixRadiansN(
		XFLOAT radianX, XFLOAT radianY, XFLOAT radianZ);
	inline Matrix33 createXYZRotationMatrixDegreesN(
		XFLOAT degreeX, XFLOAT degreeY, XFLOAT degreeZ);

	//Color functions
	//32-bit 10 bit components 2 alpha
	inline XDWORD colorEncode32_10ARGB(XWORD a, XWORD r, XWORD g, XWORD b);
	inline XDWORD colorEncode32_10ARGB(Vector4D &v);
	inline XDWORD colorEncode32_10XRGB(XWORD r, XWORD g, XWORD b);
	inline XDWORD colorEncode32_10XRGB(Vector3D &v);

	//32-bit standard
	inline XDWORD colorEncode32ARGB(XBYTE a, XBYTE r, XBYTE g, XBYTE b);
	inline XDWORD colorEncode32ARGB(Vector4D &v);
	inline XDWORD colorEncode32XRGB(XBYTE r, XBYTE g, XBYTE b);
	inline XDWORD colorEncode32XRGB(Vector3D &v);

	//16-bit standard
	inline XWORD colorEncode16ARGB(XBYTE a, XBYTE r, XBYTE g, XBYTE b);
	inline XWORD colorEncode16ARGB(Vector4D &v);
	inline XWORD colorEncode16RGB(XBYTE r, XBYTE g, XBYTE b);
	inline XWORD colorEncode16RGB(Vector3D &v);
};

extern MathFunctions MATH;
/*****************************************************************************/
inline XFLOAT MathFunctions::degreesToRadians(XFLOAT &degree)
{
	return((XFLOAT)((degree * PI) / 180.00));
}
/*****************************************************************************/
inline XFLOAT MathFunctions::radiansToDegrees(XFLOAT &radian)
{
	return((XFLOAT)((radian * 180.00) / PI));
}
/*****************************************************************************/
inline XFLOAT MathFunctions::degreesToRadiansN(XFLOAT degree)
{
	return((XFLOAT)((degree * PI) / 180.00));
}
/*****************************************************************************/
inline XFLOAT MathFunctions::radiansToDegreesN(XFLOAT radian)
{
	return((XFLOAT)((radian * 180.00) / PI));
}
/*****************************************************************************/
inline Matrix33 MathFunctions::createXRotationMatrixRadians(
	XFLOAT &radian)
{
	XFLOAT cosVal, sinVal;
    Matrix33 rotation;
	
	cosVal = (XFLOAT)cos(radian);
	sinVal = (XFLOAT)sin(radian);
	rotation.setIdentity();
	rotation[1][1] = cosVal;
	rotation[1][2] = -sinVal;
	rotation[2][1] = sinVal;
	rotation[2][2] = cosVal;

    return(rotation);
}
/*****************************************************************************/
inline Matrix33 MathFunctions::createXRotationMatrixDegrees(XFLOAT &degree)
{
	return(createXRotationMatrixRadiansN(degreesToRadians(degree)));
}
/*****************************************************************************/
inline Matrix33 MathFunctions::createYRotationMatrixRadians(XFLOAT &radian)
{
	XFLOAT cosVal, sinVal;
    Matrix33 rotation;

	cosVal = (XFLOAT)cos(radian);
	sinVal = (XFLOAT)sin(radian);
	rotation.setIdentity();
	rotation[0][0] = cosVal;
	rotation[0][2] = sinVal;
	rotation[2][0] = -sinVal;
	rotation[2][2] = cosVal;

    return(rotation);
}
/*****************************************************************************/
inline Matrix33 MathFunctions::createYRotationMatrixDegrees(XFLOAT &degree)
{
	return(createYRotationMatrixRadiansN(degreesToRadians(degree)));
}
/*****************************************************************************/
inline Matrix33 MathFunctions::createZRotationMatrixRadians(XFLOAT &radian)
{
	XFLOAT cosVal, sinVal;
    Matrix33 rotation;

	cosVal = (XFLOAT)cos(radian);
	sinVal = (XFLOAT)sin(radian);
	rotation.setIdentity();
	rotation[0][0] = cosVal;
	rotation[0][1] = -sinVal;
	rotation[1][0] = sinVal;
	rotation[1][1] = cosVal;

    return(rotation);
}
/*****************************************************************************/
inline Matrix33 MathFunctions::createZRotationMatrixDegrees(XFLOAT &degree)
{
	return(createZRotationMatrixRadiansN(degreesToRadians(degree)));
}
/*****************************************************************************/
inline Matrix33 MathFunctions::createXYZRotationMatrixRadians(
	XFLOAT &radianX, XFLOAT &radianY, XFLOAT &radianZ)
{
    Matrix33 rotation;
	XFLOAT s_x, s_y, s_z;
	XFLOAT c_x, c_y, c_z;
	XFLOAT c_x_s_y, s_x_s_y;
	Vector3D &row0 = rotation[0];
	Vector3D &row1 = rotation[1];
	Vector3D &row2 = rotation[2];

	//Setup table vars.
	s_x = (XFLOAT)sin(radianX);
	s_y = (XFLOAT)sin(radianY);
	s_z = (XFLOAT)sin(radianZ);
	c_x = (XFLOAT)cos(radianX);
	c_y = (XFLOAT)cos(radianY);
	c_z = (XFLOAT)cos(radianZ);
	c_x_s_y = c_x * s_y;
	s_x_s_y = s_x * s_y;

	row0[0] = c_y * c_z;
	row1[0] = (s_x_s_y * c_z) - (c_x * s_z);
	row2[0] = (c_x_s_y * c_z) + (s_x * s_z);

	row0[1] = c_y * s_z;
	row1[1] = (s_x_s_y * s_z) + (c_x * c_z);
	row2[1] = (c_x_s_y * s_z) - (s_x * c_z);

	row0[2] = -s_y;
	row1[2] = s_x * c_y;
	row2[2] = c_x * c_y;

    return(rotation);
}
/*****************************************************************************/
inline Matrix33 MathFunctions::createXYZRotationMatrixDegrees(
	XFLOAT &degreeX, XFLOAT &degreeY, XFLOAT &degreeZ)
{
	return(createXYZRotationMatrixRadiansN(degreesToRadians(degreeX),
		degreesToRadians(degreeY), degreesToRadians(degreeZ)));
}
	
/*****************************************************************************/
inline Matrix33 MathFunctions::createXRotationMatrixDegreesN(XFLOAT degree)
{
	return(createXRotationMatrixRadiansN(degreesToRadians(degree)));
}
/*****************************************************************************/
inline Matrix33 MathFunctions::createYRotationMatrixRadiansN(XFLOAT radian)
{
	XFLOAT cosVal, sinVal;
    Matrix33 rotation;

	cosVal = (XFLOAT)cos(radian);
	sinVal = (XFLOAT)sin(radian);
	rotation.setIdentity();
	rotation[0][0] = cosVal;
	rotation[0][2] = sinVal;
	rotation[2][0] = -sinVal;
	rotation[2][2] = cosVal;

    return(rotation);
}
/*****************************************************************************/
inline Matrix33 MathFunctions::createYRotationMatrixDegreesN(XFLOAT degree)
{
	return(createYRotationMatrixRadiansN(degreesToRadians(degree)));
}
/*****************************************************************************/
inline Matrix33 MathFunctions::createZRotationMatrixRadiansN(XFLOAT radian)
{
	XFLOAT cosVal, sinVal;
    Matrix33 rotation;

	cosVal = (XFLOAT)cos(radian);
	sinVal = (XFLOAT)sin(radian);
	rotation.setIdentity();
	rotation[0][0] = cosVal;
	rotation[0][1] = -sinVal;
	rotation[1][0] = sinVal;
	rotation[1][1] = cosVal;

    return(rotation);
}
/*****************************************************************************/
inline Matrix33 MathFunctions::createZRotationMatrixDegreesN(XFLOAT degree)
{
	return(createZRotationMatrixRadiansN(degreesToRadians(degree)));
}
/*****************************************************************************/
inline Matrix33 MathFunctions::createXYZRotationMatrixRadiansN(
	XFLOAT radianX, XFLOAT radianY, XFLOAT radianZ)
{
    Matrix33 rotation;
	XFLOAT s_x, s_y, s_z;
	XFLOAT c_x, c_y, c_z;
	XFLOAT c_x_s_y, s_x_s_y;
	Vector3D &row0 = rotation[0];
	Vector3D &row1 = rotation[1];
	Vector3D &row2 = rotation[2];

	//Setup table vars.
	s_x = (XFLOAT)sin(radianX);
	s_y = (XFLOAT)sin(radianY);
	s_z = (XFLOAT)sin(radianZ);
	c_x = (XFLOAT)cos(radianX);
	c_y = (XFLOAT)cos(radianY);
	c_z = (XFLOAT)cos(radianZ);
	c_x_s_y = c_x * s_y;
	s_x_s_y = s_x * s_y;

	row0[0] = c_y * c_z;
	row1[0] = (s_x_s_y * c_z) - (c_x * s_z);
	row2[0] = (c_x_s_y * c_z) + (s_x * s_z);

	row0[1] = c_y * s_z;
	row1[1] = (s_x_s_y * s_z) + (c_x * c_z);
	row2[1] = (c_x_s_y * s_z) - (s_x * c_z);

	row0[2] = -s_y;
	row1[2] = s_x * c_y;
	row2[2] = c_x * c_y;
    return(rotation);
}
/*****************************************************************************/
inline Matrix33 MathFunctions::createXYZRotationMatrixDegreesN(
	XFLOAT degreeX, XFLOAT degreeY, XFLOAT degreeZ)
{
	return(createXYZRotationMatrixRadiansN(degreesToRadians(degreeX),
		degreesToRadians(degreeY), degreesToRadians(degreeZ)));
}
/*****************************************************************************/
//32-bit 10 bit components 2 alpha
inline XDWORD MathFunctions::colorEncode32_10ARGB(XWORD a, XWORD r, XWORD g, XWORD b)
{
	XDWORD rt = 0;
	rt = ((a>>6)<<30) | ((r>>6)<<20) | ((g>>6)<<10) | (b>>6);
	return(rt);
}
/*****************************************************************************/
inline XDWORD MathFunctions::colorEncode32_10ARGB(Vector4D &v)
{
	XDWORD rt = 0;
	XWORD a=(XWORD)v[0];
	XWORD r=(XWORD)v[1];
	XWORD g=(XWORD)v[2];
	XWORD b=(XWORD)v[3];
	rt = ((a>>6)<<30) | ((r>>6)<<20) | ((g>>6)<<10) | (b>>6);
	return(rt);
}
/*****************************************************************************/
inline XDWORD MathFunctions::colorEncode32_10XRGB(XWORD r, XWORD g, XWORD b)
{
	XDWORD rt = 0;
	rt = ((0xffff>>6)<<30) | ((r>>6)<<20) | ((g>>6)<<10) | (b>>6);
	return(rt);
}
/*****************************************************************************/
inline XDWORD MathFunctions::colorEncode32_10XRGB(Vector3D &v)
{
	XDWORD rt = 0;
	XWORD r=(XWORD)v[0];
	XWORD g=(XWORD)v[1];
	XWORD b=(XWORD)v[2];
	rt = ((0xffff>>6)<<30) | ((r>>6)<<20) | ((g>>6)<<10) | (b>>6);
	return(rt);
}
/*****************************************************************************/
//32-bit standard
/*****************************************************************************/
inline XDWORD MathFunctions::colorEncode32ARGB(XBYTE a, XBYTE r, XBYTE g, XBYTE b)
{
	XDWORD rt = 0;
	rt = (a<<24) | (r<<16) | (g<<8) | b;
	return(rt);
}
/*****************************************************************************/
inline XDWORD MathFunctions::colorEncode32ARGB(Vector4D &v)
{
	XDWORD rt = 0;
	XBYTE a=(XBYTE)v[0];
	XBYTE r=(XBYTE)v[1];
	XBYTE g=(XBYTE)v[2];
	XBYTE b=(XBYTE)v[3];
	rt = (a<<24) | (r<<16) | (g<<8) | b;	
	return(rt);
}
/*****************************************************************************/
inline XDWORD MathFunctions::colorEncode32XRGB(XBYTE r, XBYTE g, XBYTE b)
{
	XDWORD rt = 0;
	rt = (0xff<<24) | (r<<16) | (g<<8) | b;	
	return(rt);
}
/*****************************************************************************/
inline XDWORD MathFunctions::colorEncode32XRGB(Vector3D &v)
{
	XDWORD rt = 0;
	XBYTE r=(XBYTE)v[0];
	XBYTE g=(XBYTE)v[1];
	XBYTE b=(XBYTE)v[2];
	rt = (0xff<<24) | (r<<16) | (g<<8) | b;	
	return(rt);
}
/*****************************************************************************/
//16-bit standard
inline XWORD MathFunctions::colorEncode16ARGB(XBYTE a, XBYTE r, XBYTE g, XBYTE b)
{
	XWORD rt = 0;
	rt = ((a>>4)<<12) | ((r>>4)<<8) | ((g>>4)<<4) | (b>>4);
	return(rt);
}
/*****************************************************************************/
inline XWORD MathFunctions::colorEncode16ARGB(Vector4D &v)
{
	XWORD rt = 0;
	XBYTE a=(XBYTE)v[0];
	XBYTE r=(XBYTE)v[1];
	XBYTE g=(XBYTE)v[2];
	XBYTE b=(XBYTE)v[3];
	rt = ((a>>4)<<12) | ((r>>4)<<8) | ((g>>4)<<4) | (b>>4);
	return(rt);
}
/*****************************************************************************/
inline XWORD MathFunctions::colorEncode16RGB(XBYTE r, XBYTE g, XBYTE b)
{
	XWORD rt = 0;
	rt = ((r>>3)<<11) | ((g>>2)<<5) | (b>>3);
	return(rt);
}
/*****************************************************************************/
inline XWORD MathFunctions::colorEncode16RGB(Vector3D &v)
{
	XWORD rt = 0;
	XBYTE r=(XBYTE)v[0];
	XBYTE g=(XBYTE)v[1];
	XBYTE b=(XBYTE)v[2];
	rt = ((r>>3)<<11) | ((g>>2)<<5) | (b>>3);
	return(rt);
}
/*****************************************************************************/

	
