A low-level CLI tool in Python for inspecting .NET serialized objects as per
[MS-NBRF](https://msdn.microsoft.com/en-us/library/cc236844.aspx) and dumping
their contents as XML.

The implementation is robust, but it's not 100%-complete, as it covers only
fields I've actually stumbled upon - if the code doesn't work for you, please
report an issue with a test sample so that I can add support for it.

Example usage:

```console
python3 -m net_deserializer ./test.dat
```

Example output:

```xml
<Root>
    <SerializedStreamHeader>
        <RootId>1</RootId>
        <HeaderId>-1</HeaderId>
        <MajorVersion>1</MajorVersion>
        <MinorVersion>0</MinorVersion>
    </SerializedStreamHeader>
    <BinaryMethodCall>
        <Flags>20</Flags>
        <MethodName>SendAddress</MethodName>
        <TypeName>DOJRemotingMetadata.MyServer, DOJRemotingMetadata, Version=1.0.2622.31326, Culture=neutral, PublicKeyToken=null</TypeName>
        <Args>
            <ObjectId>1</ObjectId>
            <Elements>
                <Element>{}</Element>
            </Elements>
        </Args>
    </BinaryMethodCall>
    <BinaryLibrary>
        <LibraryId>3</LibraryId>
        <LibraryName>DOJRemotingMetadata, Version=1.0.2622.31326, Culture=neutral, PublicKeyToken=null</LibraryName>
        <ClassWithMembersAndTypes>
            <ObjectId>2</ObjectId>
            <ObjectName>DOJRemotingMetadata.Address</ObjectName>
            <LibraryId>3</LibraryId>
            <Members>
                <Member>
                    <Name>Street</Name>
                    <BinaryObjectString>
                        <ObjectId>4</ObjectId>
                        <Value>One Microsoft Way</Value>
                    </BinaryObjectString>
                </Member>
                <Member>
                    <Name>City</Name>
                    <BinaryObjectString>
                        <ObjectId>5</ObjectId>
                        <Value>Redmond</Value>
                    </BinaryObjectString>
                </Member>
                <Member>
                    <Name>State</Name>
                    <BinaryObjectString>
                        <ObjectId>6</ObjectId>
                        <Value>WA</Value>
                    </BinaryObjectString>
                </Member>
                <Member>
                    <Name>Zip</Name>
                    <BinaryObjectString>
                        <ObjectId>7</ObjectId>
                        <Value>98054</Value>
                    </BinaryObjectString>
                </Member>
            </Members>
        </ClassWithMembersAndTypes>
    </BinaryLibrary>
    <MessageEnd/>
</Root>
```
