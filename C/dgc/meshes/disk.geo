// Gmsh project created on Sat Feb  9 21:39:29 2013
Point(1) = {-1.0, -1.0, 0, 1.0};
Point(2) = {-1.0, 1.0, 0, 1.0};
Point(3) = {1.0, -1.0, 0, 1.0};
Point(4) = {1.0, 1.0, 0, 1.0};
Line(1) = {2, 4};
Line(2) = {4, 3};
Line(3) = {3, 1};
Line(4) = {1, 2};
Point(5) = {0.0, 0.0, 0, 1.0};
Point(6) = {0.0, 0.5, 0, 1.0};
Point(7) = {0.5, 0.0, 0, 1.0};
Point(8) = {0.0, -0.5, 0, 1.0};
Point(9) = {-0.5, 0.0, 0, 1.0};
Circle(5) = {6, 5, 7};
Circle(6) = {7, 5, 8};
Circle(7) = {8, 5, 9};
Circle(8) = {9, 5, 6};
Line Loop(9) = {5, 6, 7, 8};
Plane Surface(10) = {9};
Line Loop(11) = {1, 2, 3, 4};
Plane Surface(12) = {9, 11};
Physical Surface(1) = {10};
Physical Surface(0) = {12};
Delete {
  Line{5, 6, 7, 8};
}
Delete {
  Line{5, 8, 7, 6};
}
Delete {
  Surface{10};
}
Delete {
  Surface{12};
}
Delete {
  Line{8};
}
Delete {
  Line{5};
}
Delete {
  Line{7};
}
Delete {
  Line{6};
}
Delete {
  Point{6};
}
Delete {
  Point{5};
}
Delete {
  Point{9};
}
Delete {
  Point{8};
}
Delete {
  Point{7};
}
Plane Surface(12) = {11};
Point(7) = {-0, 0.5, 0, 1.0};
Point(8) = {-0, 0, 0, 1.0};
Point(9) = {0.5, 0, 0, 1.0};
Point(10) = {-0, -0.5, 0, 1.0};
Point(11) = {-0.5, -0, 0, 1.0};
Circle(13) = {7, 8, 9};
Circle(14) = {9, 8, 10};
Circle(15) = {10, 8, 11};
Circle(16) = {11, 8, 7};
Line Loop(17) = {13, 14, 15, 16};
Plane Surface(18) = {17};
Plane Surface(19) = {11, 17};
