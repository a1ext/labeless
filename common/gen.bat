@echo off
..\3rdparty\protobuf-3.20.3\dist\bin\protoc --cpp_out=.\cpp --python_out=.\py rpc.proto
if %ERRORLEVEL% == 0 (
    echo [OK] generated.
    copy /Y .\py\rpc_pb2.py ..\deploy\labeless\
    echo Done
) else (
    echo Error occurred! check the output
)

pause