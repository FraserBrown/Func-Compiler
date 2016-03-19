function Fact(x : Integer) returns (res : Integer) is
  tmp : Integer;
begin
  if Eq(0,x) then
    res := 1;
  else
    tmp := Fact(Minus(x,1));
    res := Times(x,tmp);
  end if;
end Fact;

function Main () is
  res : Integer;
  inp : Integer;
begin
  inp := 4;
  res := Fact(inp);
  write res;
end Main;
