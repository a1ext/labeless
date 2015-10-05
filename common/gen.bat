..\3rdparty\protobuf-2.6.1\vsprojects\Debug\protoc --cpp_out=.\cpp --python_out=.\py rpc.proto
@copy /Y .\py\rpc_pb2.py ..\test\python\
@pause