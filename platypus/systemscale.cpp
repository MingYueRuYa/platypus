#include "SystemScale.h"

#include <Windows.h>

SystemScale::~SystemScale()
{
}

bool SystemScale::IsScale()
{
    double dpi_scale = GetScalePrecision();

    if (dpi_scale < 1.0) { return false; }

    const double kpercision = 0.000001;
    return ! (dpi_scale < 1+kpercision && dpi_scale > 1-kpercision);
}

double SystemScale::GetScalePrecision()
{
	UINT dpiY = 96;
	double nScaleFactor = 1;

	HDC hdcScreen   = ::GetDC(NULL);
	dpiY            = GetDeviceCaps(hdcScreen, LOGPIXELSY);
	nScaleFactor = MulDiv(dpiY, 100, 96) / 100.0;
	ReleaseDC(NULL, hdcScreen);
	return nScaleFactor  > 1.0 ? nScaleFactor : 1;;
}
