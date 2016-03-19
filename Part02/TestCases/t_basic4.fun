function Main () is
  res : Integer;
  inp : Integer;
begin
  inp := 4;
  res := 1;
  while Less(0,inp) loop
    res := Times(res,inp);
    inp := Minus(inp,1);
  end loop;
  write res;
end Main;
