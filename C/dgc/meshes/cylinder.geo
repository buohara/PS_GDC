Point(1) = {-0.5, -0.5, 0, 1.0};
Point(2) = {-0.5, 0.5, 0, 1.0};
Point(3) = {0.5, 0.5, 0, 1.0};
Point(4) = {0.5, -0.5, 0, 1.0};
Line(1) = {2, 3};
Line(2) = {3, 4};
Line(3) = {4, 1};
Line(4) = {1, 2};
Line Loop(5) = {1, 2, 3, 4};
Plane Surface(6) = {5};
Translate {0, 0, 2} {
  Surface{6};
}
