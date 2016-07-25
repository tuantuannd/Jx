// iColor.h: interface for the iColor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IColor_H__E4C4F444_2461_4F89_B782_A9476A8635FA__INCLUDED_)
#define AFX_IColor_H__E4C4F444_2461_4F89_B782_A9476A8635FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define ARGB(a,r,g,b) \
    ((DWORD)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))

enum eColorType{COLOR_WHITE,COLOR_BLACK,
		COLOR_RED,COLOR_GREEN,COLOR_BLUE,};

const long c_lWriteColor = 0xffffffff;

struct cColor
{
	long r,g,b,a;
	cColor(long _r,long _g,long _b,long _a = 255) : r(_r),g(_g),b(_b),a(_a){}
	cColor(): r(0), g(0), b(0), a(0) {}
	cColor(long value)
	{
		b = (BYTE)value;
		g = (BYTE)(value>>8);
		r = (BYTE)(value>>16);
		a = (BYTE)(value>>24);
	}
	DWORD Value(){return ARGB(a,r,g,b);}
	bool operator == (const cColor &cr) 
	{
		return (r == cr.r && g == cr.g && b == cr.b && a == cr.a);
	}
	cColor operator - (const cColor &cr)
	{
		cColor color(r-cr.r,g-cr.g,b-cr.b,a-cr.a);
		return color;
	}
	cColor operator + (const cColor &cr)
	{
		cColor color(r+cr.r,g+cr.g,b+cr.b,a+cr.a);
		return color;
	}
	cColor operator - (long f)
	{
		cColor cr(r-f,g-f,b-f,a);
		return cr;
	}
	cColor operator + (long f)
	{
		cColor cr(r+f,g+f,b+f,a);
		return cr;
	}
	cColor operator * (float f)
	{
		cColor cr((long)(r*f),(long)(g*f),(long)(b*f),a);
		return cr;
	}
	cColor operator / (float f)
	{
		cColor cr((long)(r/f),(long)(g/f),(long)(b/f),a);
		return cr;
	}
	void operator += (long f)
	{
		r+=f;
		g+=f;
		b+=f;
	}
	void operator += (const cColor& cr)
	{
		r+=cr.r;
		g+=cr.g;
		b+=cr.b;
		a+=cr.a;
	}
	void operator -= (long f)
	{
		r-=f;
		g-=f;
		b-=f;
	}
	void operator -= (const cColor& cr)
	{
		r-=cr.r;
		g-=cr.g;
		b-=cr.b;
		a-=cr.a;
	}
	void Format()
	{
		if (r>255)r=255; if (r<0)r=0;
		if (g>255)g=255; if (g<0)g=0;
		if (b>255)b=255; if (b<0)b=0;
		if (a>255)a=255; if (a<0)a=0;
	}
};

struct cPoint
{
	float x, y, z;

	cPoint(float _x, float _y, float _z = 0): x(_x), y(_y),z(_z) {}
	cPoint(): x(0), y(0), z(0) {}

	void operator = (const cPoint &p)
	{
		x = p.x;
		y = p.y; 
		z = p.z;
	}
	bool operator != (const cPoint &p) const
	{
		return (!(x == p.x && y == p.y && z == p.z));
	}
	bool operator == (const cPoint &p) const 
	{
		return (x == p.x && y == p.y && z == p.z);
	}
	void  operator += (const cPoint &p)
	{
		x += p.x;
		y += p.y;
		z += p.z;
	}
	void  operator -= (const cPoint &p)
	{
		x -= p.x;
		y -= p.y;
		z -= p.z;
	}
	cPoint operator + (const cPoint &p)
	{
		cPoint ret(x + p.x, y + p.y, z + p.z);
		return ret;
	}
	cPoint operator - (const cPoint &p)
	{
		cPoint ret(x - p.x, y - p.y, z - p.z);
		return ret;
	}
};

struct cPoint2
{
	cPoint p1;
	cPoint p2;
};

struct POINT2
{
	POINT p1;
	POINT p2;
};

#define fmax(a,b) ((a)>(b)?(a):(b))
#define fmin(a,b) ((a)<(b)?(a):(b))

struct cRect
{
	union
	{
		float l;
		float left;
	};
	union
	{
		float r;
		float right;
	};
	union
	{
		float t;
		float top;
	};
	union
	{
		float b;
		float bottom;
	};
	
	cRect(float _l, float _t, float _r, float _b): l(_l), r(_r), t(_t), b(_b){}
	cRect(): l(0), r(0), t(0), b(0){}
	
	cRect operator + (const cRect &p)
	{
		cRect ret(l + p.l, r + p.r, t + p.t, b + p.b);
		return ret;
	}
	cRect operator - (const cRect &p)
	{
		cRect ret(l - p.l, r - p.r, t - p.t, b - p.b);
		return ret;
	}
	void operator = (const cRect &p)
	{
		l = p.l;
		r = p.r;
		t = p.t;
		b = p.b;
	}
	
	cRect operator & (cRect &rect)
	{
		return cRect(fmax(rect.l,l),fmax(rect.t,t),fmin(rect.r,r),fmin(rect.b,b));
	}

	cRect operator | (cRect &rect)
	{
		return cRect(fmin(rect.l,l),fmin(rect.t,t),fmax(rect.r,r),fmax(rect.b,b));
	}

	bool IsEmpty(){return Width() <= 0 || Height() <= 0;}
	bool IsRectEmpty(){return IsEmpty();}
	bool IsAbsEmpty(){return Width() < 0 || Height() < 0;}
	bool IsNULL(){return l == 0 && r == 0 && t == 0 && b == 0;}

	cPoint CenterPoint(){return cPoint((l+r)/2,(t+b)/2);}
	void Offset(float x,float y)
	{
		l += x;
		r += x;
		t += y;
		b += y;
	}
	void Offset(cPoint pt)
	{
		Offset(pt.x,pt.y);
	}
	float Width() const 
	{
		return r-l;
	}
	float Height() const 
	{
		return b-t;
	}
	void FormatMin(float x, float y)
	{
		if (l < x)
			l = x;
		if (r < x)
			r = x;
		if (t < y)
			t = y;
		if (b < y)
			b = y;
	}
	void FormatMinMax(cPoint pt)
	{
		FormatMinMax(pt.x,pt.y);
	}
	void FormatMinMax(float x, float y)
	{
		if (l > x)
			l = x;
		if (r < x)
			r = x;
		if (t > y)
			t = y;
		if (b < y)
			b = y;
	}
};

typedef cPoint cSize;


#endif // !defined(AFX_IColor_H__E4C4F444_2461_4F89_B782_A9476A8635FA__INCLUDED_)
