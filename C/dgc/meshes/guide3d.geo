cl1 = 1;
Point(1) = {-1, 0.5, 0, cl1};
Point(2) = {-1, -0.5, 0, cl1};
Point(3) = {1, -0.5, 0, cl1};
Point(4) = {1, 0.5, 0, cl1};
Point(5) = {-0.9, 0.4, 0, cl1};
Point(6) = {-0.9, 0, 0, cl1};
Point(7) = {-0.2, 0, 0, cl1};
Point(8) = {-0.2, 0.2, 0, cl1};
Point(9) = {0.9, 0.2, 0, cl1};
Point(10) = {0.9, 0.4, 0, cl1};
Point(11) = {-0.1, 0.1, 0, cl1};
Point(12) = {-0.1, -0, 0, cl1};
Point(13) = {0.9, 0, 0, cl1};
Point(14) = {0.9, 0.1, 0, cl1};
Point(15) = {-0.9, -0.1, 0, cl1};
Point(16) = {-0.9, -0.4, 0, cl1};
Point(17) = {0.9, -0.4, 0, cl1};
Point(18) = {0.9, -0.3, 0, cl1};
Point(19) = {-0.1, -0.1, 0, cl1};
Point(20) = {-0.1, -0.2, 0, cl1};
Point(21) = {0.9, -0.2, 0, cl1};
Point(22) = {0.9, -0.1, 0, cl1};
Point(23) = {-0.2, -0.3, 0, cl1};
Point(24) = {-0.2, -0.1, 0, cl1};
Line(1) = {1, 2};
Line(2) = {2, 3};
Line(3) = {3, 4};
Line(4) = {4, 1};
Line(5) = {5, 6};
Line(6) = {6, 7};
Line(7) = {7, 8};
Line(8) = {8, 9};
Line(9) = {9, 10};
Line(10) = {10, 5};
Line(11) = {11, 12};
Line(12) = {12, 13};
Line(13) = {13, 14};
Line(14) = {14, 11};
Line(15) = {15, 16};
Line(16) = {16, 17};
Line(17) = {17, 18};
Line(18) = {18, 23};
Line(19) = {19, 20};
Line(20) = {20, 21};
Line(21) = {21, 22};
Line(22) = {22, 19};
Line(23) = {24, 23};
Line(24) = {24, 15};
Line Loop(30) = {4, 1, 2, 3, -9, -8, -7, -6, -5, -10, -13, -12, -11, -14, -21, -20, -19, -22, 23, -18, -17, -16, -15, -24};
Plane Surface(30) = {30};
Line Loop(31) = {10, 5, 6, 7, 8, 9};
Plane Surface(31) = {31};
Line Loop(32) = {14, 11, 12, 13};
Plane Surface(32) = {32};
Line Loop(33) = {22, 19, 20, 21};
Plane Surface(33) = {33};
Line Loop(34) = {24, 15, 16, 17, 18, -23};
Plane Surface(34) = {34};
Physical Surface(0) = {31, 32, 33, 34};
Physical Surface(1) = {30};
Extrude {0, 0, 2} {
  Surface{30, 31, 34, 33, 32};
}
