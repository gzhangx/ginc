//	Author:	Gang Zhang
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
//	
#ifndef MATRIX3D_FROM_JAVA_LABHEADER
#define MATRIX3D_FROM_JAVA_LABHEADER

#include <math.h>


#ifndef GANG_DLL_EXPORT
#define GANG_DLL_EXPORT
#endif

//	A fairly conventional 3D matrix object that can transform sets of
//	3D points and perform a variety of manipulations on the transform
class GANG_DLL_EXPORT matrix3d {
    float xx, xy, xz, xo;
    float yx, yy, yz, yo;
    float zx, zy, zz, zo;
    //Create a new unit matrix
public:
    matrix3d () {
		unit();
    }

    // Scale by f in all dimensions
    void scale(float f) {
		xx *= f;
		xy *= f;
		xz *= f;
		xo *= f;
		yx *= f;
		yy *= f;
		yz *= f;
		yo *= f;
		zx *= f;
		zy *= f;
		zz *= f;
		zo *= f;
    }

    //Scale along each axis independently
    void scale(float xf, float yf, float zf) {
		xx *= xf;
		xy *= xf;
		xz *= xf;
		xo *= xf;
		yx *= yf;
		yy *= yf;
		yz *= yf;
		yo *= yf;
		zx *= zf;
		zy *= zf;
		zz *= zf;
		zo *= zf;
    }

    //Translate the origin
    void translate(float x, float y, float z) {
		xo += x;
		yo += y;
		zo += z;
    }

// [xx  xy  xz  0]   [1   0   0  0]     [xx  xy*Ca-xz*Sa  xy*Sa+xz*Ca  0]
// [yx  yy  yz  0]   [0  Ca  Sa  0]     [yx  yy*Ca-yz*Sa  yy*Sa+yz*Ca  0]
// [zx  zy  zz  0] * [0 -Sa  Ca  0]  =  [zx  zy*Ca-zz*Sa  zy*Sa+zz*Ca  0]
// [x0  y0  z0  1]   [0   0   0  1]     [x0  y0*Ca-z0*Sa  y0*Se+z0*Ca  1]
	// rotate theta degrees about the x axis
    void xrot(double theta) {
		float ct = (float)cos(theta);
		float st = (float)sin(theta);

		float Nyx = yx * ct - zx * st;
		float Nyy = yy * ct - zy * st;
		float Nyz = yz * ct - zz * st;
		float Nyo = yo * ct - zo * st;

		float Nzx = zx * ct + yx * st;
		float Nzy = zy * ct + yy * st;
		float Nzz = zz * ct + yz * st;
		float Nzo = zo * ct + yo * st;

		yo = Nyo;
		yx = Nyx;
		yy = Nyy;
		yz = Nyz;
		zo = Nzo;
		zx = Nzx;
		zy = Nzy;
		zz = Nzz;
    }
    
	//rotate theta degrees about the y axis
    void yrot(double theta) {
		float ct = (float)cos(theta);
		float st = (float)sin(theta);

		float Nxx = xx * ct + zx * st;
		float Nxy = xy * ct + zy * st;
		float Nxz = xz * ct + zz * st;
		float Nxo = xo * ct + zo * st;

		float Nzx = zx * ct - xx * st;
		float Nzy = zy * ct - xy * st;
		float Nzz = zz * ct - xz * st;
		float Nzo = zo * ct - xo * st;

		xo = Nxo;
		xx = Nxx;
		xy = Nxy;
		xz = Nxz;
		zo = Nzo;
		zx = Nzx;
		zy = Nzy;
		zz = Nzz;
    }

    // rotate theta degrees about the z axis
    void zrot(double theta) {
		float ct = (float)cos(theta);
		float st = (float)sin(theta);

		float Nyx = yx * ct + xx * st;
		float Nyy = yy * ct + xy * st;
		float Nyz = yz * ct + xz * st;
		float Nyo = yo * ct + xo * st;

		float Nxx = xx * ct - yx * st;
		float Nxy = xy * ct - yy * st;
		float Nxz = xz * ct - yz * st;
		float Nxo = xo * ct - yo * st;

		yo = Nyo;
		yx = Nyx;
		yy = Nyy;
		yz = Nyz;
		xo = Nxo;
		xx = Nxx;
		xy = Nxy;
		xz = Nxz;
    }

	// [xx yx zx 0]  [A B C 0]     [aA+bD+cG    aB+bE+cH    aC+bF+cI    0]
	// [xy yy zy 0]  [D E F 0]     [dA+eD+fG    dB+eE+fH    dC+eF+fI    0]
	// [xz yz zz 0]  [G H I 0]  =  [gA+hD+iG    gB+hE+iH    gC+hF+iI    0]
	// [x0 y0 z0 1]  [J K L 1]     [jA+kD+lG+J  jB+kE+lH+K  jC+kF+lI+L  1]
    // Multiply this matrix by a second: M = M*R
    void mult(matrix3d rhs) {
		float lxx = xx * rhs.xx + yx * rhs.xy + zx * rhs.xz;
		float lxy = xy * rhs.xx + yy * rhs.xy + zy * rhs.xz;
		float lxz = xz * rhs.xx + yz * rhs.xy + zz * rhs.xz;
		float lxo = xo * rhs.xx + yo * rhs.xy + zo * rhs.xz + rhs.xo;

		float lyx = xx * rhs.yx + yx * rhs.yy + zx * rhs.yz;
		float lyy = xy * rhs.yx + yy * rhs.yy + zy * rhs.yz;
		float lyz = xz * rhs.yx + yz * rhs.yy + zz * rhs.yz;
		float lyo = xo * rhs.yx + yo * rhs.yy + zo * rhs.yz + rhs.yo;

		float lzx = xx * rhs.zx + yx * rhs.zy + zx * rhs.zz;
		float lzy = xy * rhs.zx + yy * rhs.zy + zy * rhs.zz;
		float lzz = xz * rhs.zx + yz * rhs.zy + zz * rhs.zz;
		float lzo = xo * rhs.zx + yo * rhs.zy + zo * rhs.zz + rhs.zo;

		xx = lxx;
		xy = lxy;
		xz = lxz;
		xo = lxo;

		yx = lyx;
		yy = lyy;
		yz = lyz;
		yo = lyo;

		zx = lzx;
		zy = lzy;
		zz = lzz;
		zo = lzo;
    }

    // Reinitialize to the unit matrix
    void unit() {
		xo = 0.0f;
		xx = 1.0f;
		xy = 0.0f;
		xz = 0.0f;
		yo = 0.0f;
		yx = 0.0f;
		yy = 1.0f;
		yz = 0.0f;
		zo = 0.0f;
		zx = 0.0f;
		zy = 0.0f;
		zz = 1.0f;
    }

	void transform(float tv[], float x,float y,float z) {
		tv[0] = (x * xx + y * xy + z * xz + xo);
		tv[1] = (x * yx + y * yy + z * yz + yo);
		tv[2] = (x * zx + y * zy + z * zz + zo);
    }

	void transform(int tv[], float x,float y,float z) {
		tv[0] = (int) (x * xx + y * xy + z * xz + xo);
		tv[1] = (int) (x * yx + y * yy + z * yz + yo);
		tv[2] = (int) (x * zx + y * zy + z * zz + zo);
    }

	void transform(int tv[], float v[]) {
		float x = v[0];
		float y = v[1];
		float z = v[2];
		tv[0] = (int) (x * xx + y * xy + z * xz + xo);
		tv[1] = (int) (x * yx + y * yy + z * yz + yo);
		tv[2] = (int) (x * zx + y * zy + z * zz + zo);
    }

    //Transform nvert points from v into tv.  v contains the input
    //coordinates in floating point.  Three successive entries in
	//the array constitute a point.  tv ends up holding the transformed
	//points as integers; three successive entries per point
    void transform(int tv[], float v[], int nvert) {
		float lxx = xx, lxy = xy, lxz = xz, lxo = xo;
		float lyx = yx, lyy = yy, lyz = yz, lyo = yo;
		float lzx = zx, lzy = zy, lzz = zz, lzo = zo;
		for (int i = (nvert-1) * 3; i>= 0;i-=3) {
			float x = v[i];
			float y = v[i + 1];
			float z = v[i + 2];
			tv[i    ] = (int) (x * lxx + y * lxy + z * lxz + lxo);
			tv[i + 1] = (int) (x * lyx + y * lyy + z * lyz + lyo);
			tv[i + 2] = (int) (x * lzx + y * lzy + z * lzz + lzo);
		}
    }
};

#endif
