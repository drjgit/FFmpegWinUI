## 何时使用EnsureSuccessStatusCode

`EnsureSuccessStatusCode`的惯用用法是，当你不想以任何特定方式处理失败情况时，简洁地验证请求是否成功。当你想快速构建客户端原型时，这尤其有用。
当您决定以特定方式处理故障情况时， **请不要**执行以下操作。

```csharp
var response = await client.GetAsync(...);
try
{
    response.EnsureSuccessStatusCode();
    // Handle success
}
catch (HttpRequestException)
{
    // Handle failure
}
```

这会抛出一个异常，只是为了立即捕获它，这没有任何意义。HttpResponseMessage `IsSuccessStatusCode` 属性就是为此目的而设的。请执行以下操作。

```csharp
var response = await client.GetAsync(...);
if (response.IsSuccessStatusCode)
{
    // Handle success
}
else
{
    // Handle failure
}
```

[参考博客](https://stackoverflow.com/questions/21097730/usage-of-ensuresuccessstatuscode-and-handling-of-httprequestexception-it-throws/)