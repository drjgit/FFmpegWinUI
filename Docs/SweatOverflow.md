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

## 错误提示："要使用将会返回“auto”的函数，必须首先定义此函数"

若要解决此问题，请包含其包含命名空间的标头。

## 如果当前操作的方法为协程实现的异步操作，那么当前函数的参数禁止传递引用

❌：Windows::Foundation::IAsyncAction StartDownloadAsync(hstring& uri, hstring& fileName); //这是错误的操作，可能引发未知的异常



