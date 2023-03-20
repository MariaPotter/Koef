#include <functional>
#include <algorithm>

#include <cmath>

#include "Tool.hpp"
#include "error.hpp"
#include "calculus.hpp"

using namespace std;

const double k1 = 1;
const double k2 = 1.5;

/*
    Функция, возвращяющая функцию, которая реализует
    большую шапочку с центром i при сетке с точками
    [0..n]
*/

function<double(double)> functional_big_f(int i, int N) {
    return [N = N, i = i](double x) -> double {
        double left = max(0., static_cast<double>(i - 1) / N);
        double right = min(1., static_cast<double>(i + 1) / N);

        if (x < left || x > right) return 0;
        else return -pow(static_cast<double>(N) * (x - static_cast<double>(i) / N), 2) + 1.;
    };
}

/*
    Функция, возвращяющая функцию, которая реализует
    маленькую шапочку с левым концом i при сетке с точками
    [0..n]
*/

function<double(double)> functional_small_f(int i, int N) {
    return [N = N, i = i](double x) -> double {
        double left = max(0., static_cast<double>(i) / N);
        double right = min(1., static_cast<double>(i + 1) / N);

        if (x < left || x > right) return 0;
        else return -pow(2. * static_cast<double>(N) * (x - (static_cast<double>(i) + 0.5) / N), 2) + 1.;
    };
}

function<double(double)> functional_f(int k, int n) {
    if (k % 2 == 0) return functional_big_f(k / 2, n);
    else return functional_small_f(k / 2, n);
}

/*
    Аналоги функций выше, реализованные как многочлены
*/

Tool tool_big_f(int i, int N) {
    double a = -N * N;
    double b = 2 * i * N;
    double c = 1 - i * i;

    return Tool({c, b, a});
}

Tool tool_small_f(int i, int N) {
    double a = -4 * N * N;
    double b = 4 * N + 8 * i * N;
    double c = -4 * i - 4 * i * i;

    return Tool({c, b, a});
}

ToolWithSupp tool_f(int k, int N) {
    if (k % 2 == 0) return ToolWithSupp(tool_big_f(k / 2, N), k / 2 - 1, k / 2 + 1, N);
    else return ToolWithSupp(tool_small_f(k / 2, N), k / 2, k / 2 + 1, N);
}

vector<vector<double> >gen_diag(int N) {
    // if (N % 2 == 1) error(-1, "N is odd");

    vector<double> a(2 * N - 1);
    vector<double> b(2 * N);
    vector<double> c(2 * N + 1);
    vector<double> d(2 * N);
    vector<double> e(2 * N - 1);

    for(int k = 0; k < 2 * N + 1; ++k) {
        if (k <= 2 * N - 2) {
            ToolWithSupp big_f = tool_f(k, N);
            ToolWithSupp small_f = tool_f(k + 2, N);

            a[k] = (big_f.derivative() * small_f.derivative()).integrate(true) + (big_f * small_f).integrate(false);
        }

        if (k <= 2 * N - 1) {
            ToolWithSupp big_f = tool_f(k, N);
            ToolWithSupp small_f = tool_f(k + 1, N);

            b[k] = (big_f.derivative() * small_f.derivative()).integrate(true) + (big_f * small_f).integrate(false);
        }

        {
            ToolWithSupp big_f = tool_f(k, N);
            ToolWithSupp small_f = tool_f(k, N);

            c[k] = (big_f.derivative() * small_f.derivative()).integrate(true) + (big_f * small_f).integrate(false);
        }

        if (k >= 1) {
            ToolWithSupp big_f = tool_f(k, N);
            ToolWithSupp small_f = tool_f(k - 1, N);

            d[k - 1] = (big_f.derivative() * small_f.derivative()).integrate(true) + (big_f * small_f).integrate(false);
        }

        if (k >= 2) {
            ToolWithSupp big_f = tool_f(k, N);
            ToolWithSupp small_f = tool_f(k - 2, N);

            e[k - 2] = (big_f.derivative() * small_f.derivative()).integrate(true) + (big_f * small_f).integrate(false);
        }
    }

    return {a, b, c, d, e};
}

vector<double> gen_f(int N, std::function<double(double)> f) {
    vector<double> ret(2 * N + 1);
    
    for(int k = 0; k < 2 * N + 1; ++k) {
        double a = static_cast<double>(k / 2 - 1) / N + (k % 2 == 0 ? 0. : 1. / N);
        double b = static_cast<double>(k / 2 + 1) / N;

        auto base = functional_f(k, N);
        auto multilpied = [base = base, f = f](double x) -> double {
            return base(x) * f(x);
        };

        ret[k] = integrate(a, b, N, multilpied);
    }
    
    return ret;
}

vector<vector<double> > matr_initial(vector<vector<double> > diag, vector<double> f) {
    auto a = diag[0];
    auto b = diag[1];
    auto c = diag[2];
    auto d = diag[3];
    auto e = diag[4];

    int N = static_cast<int>(c.size()) / 2;

    c[0] = 1;
    b[0] = 4 * N;
    a[0] = 2 * N;
    f[0] = 0;
    
    e[2 * N - 2] = 0;
    d[2 * N - 1] = 0;
    c[2 * N] = 1;
    f[2 * N] = 0;

    return {a, b, c, d, e, f};
}