fifo = io.input("/tmp/myfifo")
while true do
  data = fifo:read()
  print(data)
end
