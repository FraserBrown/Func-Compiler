function Inc(inp : Integer) returns (res : Integer) 
begin
  res := Plus(1,inp);
end Inc;

function Main () is
  val : Integer;
begin
  val := Inc(2);
end Main;
