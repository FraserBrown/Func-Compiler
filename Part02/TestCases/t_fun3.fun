function Inc(x : Integer) returns (y : Integer) 
begin
  y := Plus(x,1);
end Inc;

function IncTwice(x : Integer) returns (y : Integer) is
  a : Integer;
  b : Integer;
begin
  a := Inc(x);
  b := Inc(a);
  y := Plus(a,b);
end IncTwice;

function Main () is
  res : Integer;
begin
  res := IncTwice(2);
  write res;
end Main;
