function Test() returns (res : Integer) 
begin
  res := Plus(1,2);
end Test;

function Main () is
  val : Integer;
begin
  val := Test();
  write val;
end Main;
