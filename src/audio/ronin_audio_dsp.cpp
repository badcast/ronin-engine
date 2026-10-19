#include <cmath>
#include <vector>
#include <complex>

#include "ronin.h"
#include "ronin_audio.h"

namespace RoninEngine::Runtime
{
    namespace
    {
        constexpr double kPi = 3.14159265358979323846;

        double window_value(FFTWindow window, int i, int n)
        {
            if(n <= 1)
                return 1.0;
            const double x = static_cast<double>(i) / (n - 1);
            switch(window)
            {
            case FFTWindow::Triangle:
                return 1.0 - std::fabs(2.0 * x - 1.0);
            case FFTWindow::Hamming:
                return 0.54 - 0.46 * std::cos(2.0 * kPi * x);
            case FFTWindow::Hanning:
                return 0.5 - 0.5 * std::cos(2.0 * kPi * x);
            case FFTWindow::Blackman:
                return 0.42 - 0.5 * std::cos(2.0 * kPi * x) + 0.08 * std::cos(4.0 * kPi * x);
            case FFTWindow::BlackmanHarris:
                return 0.35875 - 0.48829 * std::cos(2.0 * kPi * x) + 0.14128 * std::cos(4.0 * kPi * x) - 0.01168 * std::cos(6.0 * kPi * x);
            case FFTWindow::Rectangular:
            default:
                return 1.0;
            }
        }

        // iterative in-place radix-2 FFT, n must be a power of two
        void fft(std::vector<std::complex<double>> &a)
        {
            const size_t n = a.size();
            for(size_t i = 1, j = 0; i < n; ++i)
            {
                size_t bit = n >> 1;
                for(; j & bit; bit >>= 1)
                    j ^= bit;
                j ^= bit;
                if(i < j)
                    std::swap(a[i], a[j]);
            }
            for(size_t len = 2; len <= n; len <<= 1)
            {
                const double ang = -2.0 * kPi / static_cast<double>(len);
                const std::complex<double> wlen(std::cos(ang), std::sin(ang));
                for(size_t i = 0; i < n; i += len)
                {
                    std::complex<double> w(1.0, 0.0);
                    for(size_t k = 0; k < len / 2; ++k)
                    {
                        const std::complex<double> u = a[i + k];
                        const std::complex<double> v = a[i + k + len / 2] * w;
                        a[i + k] = u + v;
                        a[i + k + len / 2] = u - v;
                        w *= wlen;
                    }
                }
            }
        }
    } // namespace

    bool ronin_compute_spectrum(const float *pcm, int fftSize, FFTWindow window, float *magnitudes)
    {
        if(!pcm || !magnitudes || fftSize < 2 || (fftSize & (fftSize - 1)) != 0)
            return false;

        std::vector<std::complex<double>> data(static_cast<size_t>(fftSize));
        double sumWindow = 0.0;
        for(int i = 0; i < fftSize; ++i)
        {
            const double w = window_value(window, i, fftSize);
            sumWindow += w;
            data[i] = std::complex<double>(pcm[i] * w, 0.0);
        }
        if(sumWindow <= 0.0)
            return false;

        fft(data);

        const int bins = fftSize / 2;
        const double scale = 2.0 / sumWindow;
        for(int k = 0; k < bins; ++k)
        {
            // DC component is not mirrored, so it must not be doubled
            const double s = (k == 0) ? scale * 0.5 : scale;
            magnitudes[k] = static_cast<float>(std::abs(data[k]) * s);
        }
        return true;
    }
} // namespace RoninEngine::Runtime
