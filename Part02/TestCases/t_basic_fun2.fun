function Fact(inp : Integer) returns (res : Integer) 
begin
  res := 1;
  while Less(0,inp) loop
    res := Times(res,inp);
    inp := Minus(inp,1);
  end loop;
end Fact;

function Main () is
  res : Integer;
  inp : Integer;
begin
  inp := 4;
  res := Fact(inp);
  write res;
end Main;
