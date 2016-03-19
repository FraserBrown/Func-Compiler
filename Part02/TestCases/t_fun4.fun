
function H() returns (y : Integer) is
  x : Integer;
  z : Integer;
begin
  x := 3;
  y := x;
end H;

function G() returns (y : Integer) is
  x : Integer;
  z : Integer;
begin
  x := 2;
  z := H();
  y := x;
end G;

function F() returns (y : Integer) is
  x : Integer;
  z : Integer;
begin
  x := 1;
  z := G();
  y := x;
end F;

function Main () is
  res : Integer;
begin
  res := F();
  write res;
end Main;
