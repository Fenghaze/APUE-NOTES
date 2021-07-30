线程池并发模型中，程序逻辑是：

- ①main线程负责监听lfd和所有新客户连接的cfd；
- ②当lfd可读时，除了accept新客户，还需要调用`HttpServer`的`init()`方法来初始化客户信息
- ②当cfd可读时，调用`HttpServer`的`read()`方法接收客户发送的http请求报文、调用`threadpool.append()`向任务队列中添加任务
  - threadpool中维护一个只负责处理IO的任务队列，当任务队列中有任务时，等待的工作线程就来抢占任务，并执行`HttpServer`的`process()`方法



# HttpServer

HTTP服务类，封装HttpRequset类和HttpResponse类的核心方法，暴露给用户

`init()`：将cfd注册到epfd上，客户数量+1

`read()`：封装`HttpRequest`的`read()`方法

`write()`：封装`HttpResponse`的`write()`方法

`process()`：封装`HttpRequest`的`process_request()`方法和`HttpResponse`的`process_response(HTTP_CODE ret)`方法，首先调用`process_request()`解析http request获得处理结果`read_ret`，然后将`read_ret`传递到`process_response(HTTP_CODE ret)`中生成http response



# HttpRequset

http request（GET）data格式如下：`\r\n`标志一行的结束

```http
GET /phpstudy2015-6/ HTTP/1.1		←请求行
Host: www.cnblogs.com				←请求头
User-Agent: Mozilla/5.0 (Windows; U; Windows NT 6.0; en-US; rv:1.9.0.10) ←请求头Gecko/2009042316 Firefox/3.0.10	
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
...
Keep-Alive: 300
Connection: keep-alive
If-Modified-Since: Sat, 06 May 2017 12:05:41 GMT
```

程序逻辑是：

- ①`HttpServer::read()`调用`read()`读取客户端发送的request data保存到read_buf中
- ②调用`check_line()`来检查read_buf中是否获取了一个完整的行
- ③如果是完整的行，根据`m_state`客户请求的状态解析行数据（请求头or请求行or请求体）
- ④如果不是完整的行，则重复②

`read()`：调用系统调用函数`recv()`读取request data

`check_line()`：检查是否是完整的一行，http请求中遇到`\r\n`代表是完整的一行，涉及到的变量有`m_checked_idx`和`m_read_idx`

`process_request()`：只要**读取到一个完整的行**就推动状态机，先解析请求行`parse_requestline()`，再解析请求头`parse_header()`、如果是POST请求则最后还要解析请求体`parse_content()`

`parse_content()`：检查请求体的完整性，并将请求体的内容保存起来

`do_request()`：**处理GET、POST请求，根据请求的url地址，将要不同的HTML资源文件映射到内存地址`m_file_address`中，当执行http响应时，通过内存地址发送HTML资源文件给客户端**



# HttpResponse

http response 消息格式如下：`\r\n`标志一行的结束

```http
HTTP/1.1 200 OK				←响应行：HTTP版本 状态码 状态码的文本描述\r\n
Bdpagetype: 2				←响应头
Bdqid: 0xa03365550001db15
Cache-Control: private
Connection: keep-alive
Content-Encoding: gzip
Content-Type: text/html;charset=utf-8
...
Transfer-Encoding: chunked
html文件  			      ←响应主体
```

程序逻辑是：

- ①服务端一旦接收到了http request，就会返回http response，是成对出现的，因此把HttpRequest的实例作为HttpResponse的成员，方便后续使用
  - 实现`HttpServer::process()`：首先调用`process_request()`解析http request获得处理结果`read_ret`，然后将`read_ret`传递到`process_response(HTTP_CODE ret)`中生成http response
- ②`m_iv`是一个IO向量，可以用于保存多个缓冲区的数据：`m_write_buf`是保存响应行和响应头的缓冲区，`m_file_address`是保存html的内存缓冲区，这两个缓冲区的数据组合成最终的response data

`set_response()`：格式化响应消息

`process_write(HTTP_CODE ret)`：根据ret生成不同的http response，当`ret==FILE_REQUEST`时，调用`set_status_line()`生成响应行，调用`set_headers()`生成响应头，把`m_wirte_buf`的内容保存到IO向量`m_iv[0]`中，把内存映射中的HTML资源文件内容保存到IO向量`m_iv[1]`中，更新待发送字节数`bytes_to_send`和缓冲区个数`m_iv_count`

`write()`：调用系统调用函数`writev()`，发送IO向量`m_iv`的`m_iv_count`个缓冲区的数据给cfd，发送完毕或失败则调用`unmap()`解除HTML资源文件的内存映射

