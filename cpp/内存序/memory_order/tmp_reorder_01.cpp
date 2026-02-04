#include <iostream>
#include <thread>


static int x = 0, y = 0;
static int a = 0, b = 0;

int main() {
	std::size_t iteration = 0;
	while (true) {
		++iteration;
		x = y = a = b = 0;

		std::thread one([] {
			a = 1;
			x = b;
		});

		std::thread other([] {
			b = 1;
			y = a;
		});

		one.join();
		other.join();

		if (x == 0 && y == 0) {
			std::cout << "hit at iteration=" << iteration
					  << " (x,y)= (" << x << "," << y << ")"
					  << " (a,b)= (" << a << "," << b << ")" << std::endl;
			break;
		}

		if (iteration % 1000000 == 0) {
			std::cout << "iteration=" << iteration
					  << " (x,y)= (" << x << "," << y << ")" << std::endl;
		}
	}

	return 0;
}


/*
1. 在不同的架构测试的结果不同
2. 正常情况下，第一次执行时间稍长(命中成本)，第二次、第三次会低很多

*/


/*
// macos 测试结果：
zhangbuda@localhost 2026Q1_ % ./tmp_0204_01 
Reordering detected after 1082 iterations: x=0, y=0
zhangbuda@localhost 2026Q1_ % ./tmp_0204_01
Reordering detected after 849 iterations: x=0, y=0
zhangbuda@localhost 2026Q1_ % ./tmp_0204_01
Reordering detected after 709 iterations: x=0, y=0
zhangbuda@localhost 2026Q1_ % ./tmp_0204_01
Reordering detected after 686 iterations: x=0, y=0
zhangbuda@localhost 2026Q1_ % ./tmp_0204_01
Reordering detected after 1905 iterations: x=0, y=0
zhangbuda@localhost 2026Q1_ % ./tmp_0204_01
Reordering detected after 1101 iterations: x=0, y=0
zhangbuda@localhost 2026Q1_ % ./tmp_0204_01
Reordering detected after 875 iterations: x=0, y=0
zhangbuda@localhost 2026Q1_ % ./tmp_0204_01
Reordering detected after 590 iterations: x=0, y=0
zhangbuda@localhost 2026Q1_ % ./tmp_0204_01
Reordering detected after 753 iterations: x=0, y=0
zhangbuda@localhost 2026Q1_ % ./tmp_0204_01
Reordering detected after 1008 iterations: x=0, y=0
zhangbuda@localhost 2026Q1_ % ./tmp_0204_01
Reordering detected after 1028 iterations: x=0, y=0
zhangbuda@localhost 2026Q1_ % ./tmp_0204_01
Reordering detected after 689 iterations: x=0, y=0
zhangbuda@localhost 2026Q1_ % ./tmp_0204_01
Reordering detected after 843 iterations: x=0, y=0
zhangbuda@localhost 2026Q1_ % ./tmp_0204_01
Reordering detected after 651 iterations: x=0, y=0
zhangbuda@localhost 2026Q1_ % ./tmp_0204_01
Reordering detected after 828 iterations: x=0, y=0


// amd linux 测试结果:
[root@zhangbuda7788 c_cpp_]#  /tmp/tmp_01
hit at iteration=6228 (x,y)= (0,0) (a,b)= (1,1)
[root@zhangbuda7788 c_cpp_]#  /tmp/tmp_01
hit at iteration=1084 (x,y)= (0,0) (a,b)= (1,1)
[root@zhangbuda7788 c_cpp_]#  /tmp/tmp_01
hit at iteration=527 (x,y)= (0,0) (a,b)= (1,1)
[root@zhangbuda7788 c_cpp_]#  /tmp/tmp_01
hit at iteration=724 (x,y)= (0,0) (a,b)= (1,1)
[root@zhangbuda7788 c_cpp_]#  /tmp/tmp_01
hit at iteration=11 (x,y)= (0,0) (a,b)= (1,1)
[root@zhangbuda7788 c_cpp_]#  /tmp/tmp_01
hit at iteration=501 (x,y)= (0,0) (a,b)= (1,1)
*/

