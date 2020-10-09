#ifndef EXT_OPENCVEXTENSION_H
#define	EXT_OPENCVEXTENSION_H

#include <opencv2/core/core.hpp>

namespace cv {

	const Scalar BLACK(0, 0, 0);
	const Scalar RED(255, 0, 0);
	const Scalar GREEN(0, 255, 0);
	const Scalar BLUE(0, 0, 255);
	const Scalar WHITE(255, 255, 255);

	const Scalar ZERO(0);

	void normalization(InputArray _a, OutputArray _b);

	void meanFilter(InputArray _a, OutputArray _b, size_t n = 3, Size s = Size(5, 5));

	void interpolate(const Rect& a, const Rect& b, Rect& c, double p);

	template<typename T>
	void detrend(InputArray _z, OutputArray _r, int lambda = 10) 
	{
		Mat z = _z.total() == static_cast<size_t>(_z.size().height) ? _z.getMat() : _z.getMat().t();
		if (z.total() < 3) {
			z.copyTo(_r);
		}
		else {
			int t = z.total();
			Mat i = Mat::eye(t, t, z.type());
			Mat d = Mat(Matx<T, 1, 3>(1, -2, 1));
			Mat d2Aux = Mat::ones(t - 2, 1, z.type()) * d;
			Mat d2 = Mat::zeros(t - 2, t, z.type());
			for (int k = 0; k < 3; k++) {
				d2Aux.col(k).copyTo(d2.diag(k));
			}
			Mat r = (i - (i + lambda * lambda * d2.t() * d2).inv()) * z;
			r.copyTo(_r);
		}
	}

	template<typename T>
	int countZeros(InputArray _a) 
	{
		int count = 0;
		if (_a.total() > 0) {
			Mat a = _a.getMat();
			T last = a.at<T>(0);
			for (int i = 1; i < a.total(); i++) {
				T current = a.at<T>(i);
				if ((last < 0 && current >= 0) || (last > 0 && current <= 0)) {
					count++;
				}
				last = current;
			}
		}
		return count;
	}
}

#endif	

