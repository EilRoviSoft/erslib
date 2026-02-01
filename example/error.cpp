// std
#include <iostream>
#include <limits>

// ers
#include <erslib/error/include.hpp>

namespace {
	ers::Result<double> safe_divide(double l, double r) {
		if (r < std::numeric_limits<double>::epsilon())
			return ers::Unexpected<ers::Error>(ers::Severity::Crit, "math_error", "Divided by 0");
		return l / r;
	}
}

int main() {
	double n1, n2;
	std::cin >> n1 >> n2;

	if (auto r = safe_divide(n1, n2); r)
		std::cout << *r;
	else
		std::cout << r.error().to_string();

	return 0;
}
