function Main () is
  res : Integer;
  inp : Integer;
begin
  inp := 5;
  res := 0;
  while Less(0,inp) loop
    res := Plus(res,inp);
    inp := Minus(inp,1);
  end loop;
  write res;
end Main;
