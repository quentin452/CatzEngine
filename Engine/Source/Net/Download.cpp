/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************

   For 'Download' 'Memb' is good for storing data,
      because speed of accessing data is not important,
      however files can be big and Memb only allocates new chunks of data,
      while Memc requires reallocation and copying.

/******************************************************************************/
#define BUF_SIZE (64*1024) // 64 KB
/******************************************************************************/
// DOWNLOAD
/******************************************************************************/
static void EncodeChar(Str8 &text, Char8 c)
{
   text+='%';
   text+=Digits16[Unsigned(c)>>4];
   text+=Digits16[Unsigned(c)&15];
}
static Bool AppendUrlPath(Str8 &text, C Str8 &path)
{
   Bool has_params=false;
   text.reserve(text.length()+path.length());
   FREPA(path)
   {
      Char8 c=path[i];
      if(Unsigned(c)<=32 || c=='#' || c=='%' || Unsigned(c)>=127)EncodeChar(text, c);else // these are the only symbols that need to be replaced with %XX hex code
      if(c=='\\') // use Unix style paths
      {
         text+='/';
      }else
      {
         if(c=='?')has_params=true;
         text+=c;
      }
   }
   return has_params;
}
static void AppendParam(Str8 &text, C TextParam &param)
{
 C Str8 &name=UTF8(param.name);
   FREPA(name) // set name
   {
      Char8 c=name[i];
      if(Unsigned(c)<=32 || c=='#' || c=='%' || c=='=' || c=='&' || Unsigned(c)>=127)EncodeChar(text, c); // these are the only symbols that need to be replaced with %XX hex code
      else text+=c;
   }
   text+='=';
 C Str8 &value=UTF8(param.value);
   FREPA(value) // set value
   {
      Char8 c=value[i];
      if(Unsigned(c)<=32 || c=='#' || c=='%' || c=='&' || c=='+' || Unsigned(c)>=127)EncodeChar(text, c); // these are the only symbols that need to be replaced with %XX hex code
      else text+=c;
   }
}
static void AppendParamBin(Str8 &text, C TextParam &param) // don't convert 'param.value'
{
 C Str8 &name=UTF8(param.name);
   FREPA(name) // set name
   {
      Char8 c=name[i];
      if(Unsigned(c)<=32 || c=='#' || c=='%' || c=='=' || c=='&' || Unsigned(c)>=127)EncodeChar(text, c); // these are the only symbols that need to be replaced with %XX hex code
      else text+=c;
   }
   text+='=';
 C Str  &value=param.value;
   FREPA(value) // set value
   {
      Char8 c=value[i];
      if(Unsigned(c)<=32 || c=='#' || c=='%' || c=='&' || c=='+' || Unsigned(c)>=127)EncodeChar(text, c); // these are the only symbols that need to be replaced with %XX hex code
      else text+=c;
   }
}
static void AppendName(Str8 &text, C Str &name)
{
 C Str8 &name8=UTF8(name); FREPA(name8)
   {
      Char8 c=name8[i];
      if(c=='"')text+="\\\"";else
      if(Unsigned(c)>=32)text+=c;
   }
}
static Str8 GetRange(C Download &down)
{
   if(down._size>0              )return S8+"bytes="+down._offset+'-'+(down._offset+down._size-1);
   if(down._size && down._offset)return S8+"bytes="+down._offset+'-';
                                 return S8;
}
static Str8 GetHeaders(C Str8 &url, CChar8 *request)
{
   CChar8 *name    =SkipHttp(url);
   Str8    headers =S8+request+" /"+GetStartNot(name)+" HTTP/1.1\r\nHost: "+_GetStart(name)+"\r\n";
           headers+="Connection: close\r\n"; // connection will be closed after receiving all data
#if WINDOWS
   headers+="User-Agent: " ENGINE_NAME " Windows\r\n";
#elif MAC
   headers+="User-Agent: " ENGINE_NAME " Mac\r\n";
#elif LINUX
   headers+="User-Agent: " ENGINE_NAME " Linux\r\n";
#elif ANDROID
   headers+="User-Agent: " ENGINE_NAME " Android\r\n";
#elif IOS
   headers+=(D.smallSize() ? "User-Agent: " ENGINE_NAME " iPhone\r\n" : "User-Agent: " ENGINE_NAME " iPad\r\n");
#endif
   return headers;
}
static CChar8* FileSuffix   () {return "\r\n";}
static Int     FileSuffixLen() {return 2;} // Length(FileSuffix())
/******************************************************************************/
#if WEB
struct JSDownload
{
   Download *download;
};
Memx<JSDownload> JSDownloads;
static SyncLock  JSDownloadsLock;
static void OnProgress(Ptr user, Int code, Int done, Int total, UInt timestamp) // TODO: Warning: this is a 32-bit timestamp
{
   if(JSDownloads.elms()) // check in case 'JSDownloads' destructor was already called
   {
      JSDownload &js_download=*(JSDownload*)user;
      SyncLocker lock(JSDownloadsLock);
      if(Download *download=js_download.download)
      {
         download->_code=code;
         if(timestamp)download->_modif_time.from1970s(timestamp);
         if(download->_size) // expecting size
         {
            download->_done=done;
            download->_size=total;
         }
         download->state(DWNL_DOWNLOAD); // set at the end
      }
   }
}
static void OnDone(Ptr user, Int code, CPtr data, Int data_size, Int content_length, Int content_range, UInt timestamp) // TODO: Warning: this is a 32-bit timestamp
{
   if(JSDownloads.elms()) // check in case 'JSDownloads' destructor was already called
   {
      JSDownload &js_download=*(JSDownload*)user;
      {
         SyncLocker lock(JSDownloadsLock);
         if(Download *download=js_download.download)
         {
            download->_code=code;
            if(timestamp)download->_modif_time.from1970s(timestamp);
            if(content_length<0)content_length=data_size;
            if(download->_size) // expecting size
            {
               download->_size=content_length;
               download->_done=data_size;
               Copy(download->_data=Alloc(data_size), data, data_size);
            }else
            {
               download->_total_size=content_length;
            }
            if(download->_total_size<0)download->_total_size=((content_range>=0) ? content_range : download->_size);
            download->_js_download=null; // unlink
            download->finish(); // !! set at the end !!
         }
      }
      JSDownloads.removeData(&js_download);
   }
}
static void OnError(Ptr user, Int code)
{
   if(JSDownloads.elms()) // check in case 'JSDownloads' destructor was already called
   {
      JSDownload &js_download=*(JSDownload*)user;
      {
         SyncLocker lock(JSDownloadsLock);
         if(Download *download=js_download.download)
         {
            download->_code=code;
            download->_js_download=null; // unlink
            download->state(DWNL_ERROR); // !! set at the end !!
         }
      }
      JSDownloads.removeData(&js_download);
   }
}
#endif
/******************************************************************************/
void Download::parse(Byte *data, Int size)
{
   if(data)for(; size>0; )switch(_parse)
   {
      case PARSE_DATA_SIZE:
      {
         for(; size>0; )
         {
            Byte c=*data++; size--;
            if(c=='\n')
            {
              _parse=(_expected_size ? PARSE_DATA : PARSE_END);
               break;
            }
            Int i=CharInt(c);
            if(InRange(i, 16))_expected_size=_expected_size*16+i;else continue;
         }
      }break;

      case PARSE_DATA:
      {
         Int left =Min(size, _expected_size);
         Int start=_memb.addNum(left);
         REP(left)_memb[start+i]=data[i];
         size         -=left;
         data         +=left;
        _done         +=left;
        _expected_size-=left;
         if(!_expected_size)_parse=PARSE_SKIP_LINE;
      }break;

      case PARSE_SKIP_LINE:
      {
         for(; size>0; )
         {
            Byte c=*data++; size--;
            if(c=='\n')
            {
              _parse=PARSE_DATA_SIZE;
               break;
            }
         }
      }break;

      default: return;
   }
}
/******************************************************************************/
#if !WEB
#if SUPPORT_MBED_TLS
static int DownloadSendFunc(Ptr ctx, C Byte *buf, size_t len)
{
   Download &d=*(Download*)ctx; Int r=d._socket.Socket::send(buf, (Int)len); if(r<0)return Socket::WouldBlock() ? MBEDTLS_ERR_SSL_WANT_WRITE : MBEDTLS_ERR_NET_SEND_FAILED;
   d._total_sent+=r;
   return r;
}
static int DownloadReceiveFunc(Ptr ctx, Byte *buf, size_t len)
{
   Download &d=*(Download*)ctx; Int r=d._socket.Socket::receive(buf, (Int)len); if(r<0)return Socket::WouldBlock() ? MBEDTLS_ERR_SSL_WANT_READ : MBEDTLS_ERR_NET_RECV_FAILED;
   d._total_rcvd+=r;
   return r;
}
static int DownloadReceiveFuncTimeout(Ptr ctx, Byte *buf, size_t len, uint32_t timeout)
{
   Download &d=*(Download*)ctx; if(!d._socket.wait(timeout ? Min(timeout, INT_MAX) : INT_MAX))return MBEDTLS_ERR_SSL_TIMEOUT; // Min to INT_MAX because 'wait' expected Int, if 'timeout' is zero, then use INT_MAX because MBED TLS treats this as unlimited time
   return DownloadReceiveFunc(ctx, buf, len);
}
static Bool Again(Int r, Download &d)
{
   // every time wait only a short amount of time so we can check '_thread.wantStop' frequently, return true regardless of 'flush'/'wait' result so we can continue making those short checks until download was requested to be stopped
   switch(r)
   {
      case MBEDTLS_ERR_SSL_WANT_READ :
      case MBEDTLS_ERR_SSL_TIMEOUT   : if(!d._thread.wantStop()){d._socket.wait (DOWNLOAD_WAIT_TIME); return true;} break;

      case MBEDTLS_ERR_SSL_WANT_WRITE: if(!d._thread.wantStop()){d._socket.flush(DOWNLOAD_WAIT_TIME); return true;} break;
   }
   return false;
}
#endif
Int Download::send(CPtr data, Int size)
{
#if SUPPORT_MBED_TLS
   if(_socket._secure)
   {
   again:
      Int r=mbedtls_ssl_write(_socket._secure, (Byte*)data, size); if(Again(r, T))goto again;
      return r;
   }
#endif
   Int r=_socket.Socket::send(data, size); if(r>0)_total_sent+=r; return r;
}
Int Download::receive(Ptr data, Int size)
{
#if SUPPORT_MBED_TLS
   if(_socket._secure)
   {
   again:
      Int r=mbedtls_ssl_read(_socket._secure, (Byte*)data, size); if(Again(r, T))goto again;
      return r;
   }
#endif
   Int r=_socket.Socket::receive(data, size); if(r>0)_total_rcvd+=r; return r;
}
static Bool DownloadFunc(Thread &thread) {return ((Download*)thread.user)->func();}
       Bool Download::func()
{
   Byte data[BUF_SIZE];
   switch(state())
   {
      case DWNL_CONNECTING:
      {
         // initialize connection
         if(!_socket.is())
         {
            SockAddr addr;
            if(!hasAddrsHeader())
            {
              _flags|=HAS_ADDRS_HEADER;
               CChar8 *url=_url_full(); Int port;
               if(StartsPath(url, "https://"))
               {
                  url+=8; port=443; // 443 = default HTTPS port
                  if(!_socket.secure(url))return error();
               #if SUPPORT_MBED_TLS
                  mbedtls_ssl_set_bio(_socket._secure, this, DownloadSendFunc, DownloadReceiveFunc, DownloadReceiveFuncTimeout);
               #endif
               }else
               if(StartsPath(url, "http://"))
               {
                  url+=7; port=80; // 80 = default HTTP port
               }else
                  return error();

               Char8 temp[MAX_LONG_PATH], *url_start=_GetStart(url, temp);
               if(Char8 *url_port=TextPos(url_start, ':')) // check for port override, example URL "http://www.example.com:8080/path/"
               {
                  port=TextInt(url_port+1); // ":8080" +1 to skip ':'
                 *url_port='\0'; // have to remove port specifier because 'GetHostAddresses' will fail
               }

               Memt<SockAddr>   addresses;
               GetHostAddresses(addresses, url_start, port);
               if(!addresses.elms())return error();
               addresses.reverseOrder(); // recommended order for processing is from start to end, however since we're going to use 'pop' then reverse order
               addr=addresses.pop(); _addrs=addresses; // normally we should get just 1 address, so pop it first hoping that 'addresses' would become empty and copying to '_addrs' won't require memory allocation
            }else
            {
               if(!_addrs.elms())return error();
               addr=_addrs.pop();
            }
            if(   !_socket.createTcp(addr))return error();
            switch(_socket.connect  (addr, DOWNLOAD_WAIT_TIME))
            {
               case Socket::IN_PROGRESS: break;
               case Socket::CONNECTED  : FlagDisable(_flags, HAS_ADDRS_HEADER); state(DWNL_AUTH); goto auth;
               default                 : _socket.del(); break;
            }
         }else
         {
            if(_socket.any(DOWNLOAD_WAIT_TIME)){FlagDisable(_flags, HAS_ADDRS_HEADER); state(DWNL_AUTH); goto auth;}
            if(_socket.connectFailed())_socket.del();
         }
      }break;

      case DWNL_AUTH: auth:
      {
         switch(_socket.handshake())
         {
            case SecureSocket::OK        : ok: state(DWNL_SENDING); goto sending;
            case SecureSocket::NEED_FLUSH: _socket.flush(DOWNLOAD_WAIT_TIME); return true; // flush quickly and return regardless of result, to check if thread want to stop
            case SecureSocket::NEED_WAIT : _socket.wait (DOWNLOAD_WAIT_TIME); return true; // wait  quickly and return regardless of result, to check if thread want to stop
            case SecureSocket::BAD_CERT  : _flags|=AUTH_FAILED; if(_flags&AUTH_IGNORE)goto ok; goto error;
            default                      : error: return error(); // error
         }
      }break;

      case DWNL_SENDING: sending:
      {
         // send message
         Int left=_send.length()-_send_pos; if(left>0)
         {
            if(_socket.flush(DOWNLOAD_WAIT_TIME))
            {
               Int sent=send(_send()+_send_pos, left); if(sent<=0)return error();
              _send_pos+=sent;
               if(_send_pos>=_send.length()){_send_pos=0; _send.clear();} // reached the end
            }
         }else
         if(InRange(_file_i, _files))
         {
            HTTPFile &file=_files[_file_i];
            Long left=file.Send(); if(left>0) // file data
            {
               if(_socket.flush(DOWNLOAD_WAIT_TIME))
               {
                  Int buf_left=Min(BUF_SIZE, left); if(!file.getFast(data, buf_left))return error();
                  Int sent=send(data, buf_left); if(sent<=0)return error();
                                       _sent+=sent;
                  if( file.send>=0)file.send-=sent; // adjust because 'file.pos' gets adjusted too, but only if manually specified, if not specified then keep as unlimited in case user will reuse this 'file' again by adjusting 'file.pos' only
                  if(!file.skip(sent-buf_left))return error(); // go back to unsent data
               }
            }else
            {
               if(InRange(++_file_i, _files)) // have next file
               {
                 _send =FileSuffix();        // end   previous
                 _send+=fileHeader(_file_i); // start next
                  goto sending;
               }
               goto footer;
            }
         }else
         {
         footer:
            if(_footer.is()){Swap(_send, _footer); goto sending;}
            state(DWNL_DOWNLOAD); goto downloading;
         }
      }break;

      case DWNL_DOWNLOAD: downloading:
      {
         if(_socket.wait(DOWNLOAD_WAIT_TIME))
         {
            if(!hasAddrsHeader()) // download header
            {
               Byte *rest;
               Int   rcvd=receive(data, BUF_SIZE); if(rcvd<=0)return error();
              _header.reserve(_header.length()+rcvd);
               FREP(rcvd)
               {
                 _header+=Char8(data[i]);
                  if(_header.last()=='\n' && _header[_header.length()-2]=='\r' && _header[_header.length()-3]=='\n' && _header[_header.length()-4]=='\r') // "\r\n\r\n" marks the end of the header
                  {
                    _flags|=HAS_ADDRS_HEADER;
                     i++;
                     rest =data+i;
                     rcvd-=i;
                     break;
                  }
               }
               if(hasAddrsHeader())
               {
                  //  example header= "HTTP/1.1 200 OK"
                  if(!Starts(_header, "HTTP/1.1", false, WHOLE_WORD_STRICT))return error();
          C Int http_len=8; // Length("HTTP/1.1");

                  if(CChar8 *t=TextPos(_header, "Location:")) // check if it's a redirect (check this as first before all other data)
                  {
                     Int line =TextPosI(t, "\r\n");
                     if( line>=0)ConstCast(t)[line]=0;
                     CChar8 *url=_SkipWhiteChars(t+9); // operate on 'url' before clearing the '_header'. Length("Location:")==9
                     if(!Is(url))return error(); // no path specified
                     if(StartsPath(url, "http://") || StartsPath(url, "https://")) // if this is a full path then just use it
                     {
                        AppendUrlPath(_url_full.clear(), url);
                     }else
                     { // relative path
                        if(url[0]=='/') // start of the domain
                        {
                           Int pos =TextPosIN(_url_full, '/', 2); // find the 3rd slash in the url "http://www.esenthel.com/index.php"
                           if( pos>=0)_url_full.clip(pos); // remove the slash and everything after it, leaving "http://www.esenthel.com"
                           AppendUrlPath(_url_full, url);
                        }else // relative to current location
                        {
                           FREPA(_url_full)if(_url_full[i]=='?'){_url_full.clip(i  ); break;} // first remove any parameters that were specified
                            REPA(_url_full)if(_url_full[i]=='/'){_url_full.clip(i+1); break;} // remove base name, but keep the tail slash
                           AppendUrlPath(_url_full, url);
                        //_url_full=NormalizePath(_url_full); // this is not needed because the server will handle this properly
                        }
                     }
                     FlagDisable(_flags, HAS_ADDRS_HEADER);
                    _header.clear();
                    _socket.del  (); // unsecure and delete the socket because we will need to reconnect to a different address
                    _addrs .clear();
                    _footer.clear();
                    _send_pos=0;

                         _send =GetHeaders(_url_full, (_size==0) ? "HEAD" : "GET");
                     Str8 bytes=GetRange  (T);
                     if(  bytes.is())_send+=S8+"Range: "+bytes+"\r\n";
                    _send+="\r\n";
                     state(DWNL_CONNECTING);
                     break;
                  }

                 _code=TextInt(_header()+(http_len+1)); // +1 to skip space
                  // !! do not stop here if error code failed, instead continue downloading all data, and set DWNL_STATE at the end based on the code, as there may be some cases in which we want to process output even for error codes !!

                  Long content_length=-1; if(CChar8 *t=TextPos(_header, "Content-Length:"))content_length=TextLong(t+Length("Content-Length:"));
                  if(_size)_size=content_length;else _total_size=content_length;
                  if(_total_size<0)if(CChar8 *t=TextPos(TextPos(_header, "Content-Range:" ), '/'))_total_size=TextLong(t+1);else _total_size=_size;

                  if(CChar8 *t=TextPos(_header, "Last-Modified:" ))
                  {
                     t+=Length("Last-Modified:"); // Thu, 24 Mar 2011 18:35:38 GMT
                     if(t=_SkipChar(TextPos(t, ',')))
                     {
                        CalcValue val; if(t=_SkipWhiteChars(TextValue(t, val)))if(val.type)
                        {
                          _modif_time.day=val.asInt();
                           if(Starts(t, "Jan"))_modif_time.month= 1;else
                           if(Starts(t, "Feb"))_modif_time.month= 2;else
                           if(Starts(t, "Mar"))_modif_time.month= 3;else
                           if(Starts(t, "Apr"))_modif_time.month= 4;else
                           if(Starts(t, "May"))_modif_time.month= 5;else
                           if(Starts(t, "Jun"))_modif_time.month= 6;else
                           if(Starts(t, "Jul"))_modif_time.month= 7;else
                           if(Starts(t, "Aug"))_modif_time.month= 8;else
                           if(Starts(t, "Sep"))_modif_time.month= 9;else
                           if(Starts(t, "Oct"))_modif_time.month=10;else
                           if(Starts(t, "Nov"))_modif_time.month=11;else
                           if(Starts(t, "Dec"))_modif_time.month=12;
                           if(_modif_time.month)if(t=TextValue(TextPos(t, ' '), val))if(val.type)
                           {
                             _modif_time.year=val.asInt();
                              if(t=TextValue(t, val))if(val.type)
                              {
                                _modif_time.hour=val.asInt();
                                 if(*t++==':')if(t=TextValue(t, val))if(val.type)
                                 {
                                   _modif_time.minute=val.asInt();
                                    if(*t++==':')
                                    {
                                       TextValue(t, val);
                                       if(val.type)_modif_time.second=val.asInt();
                                    }
                                 }
                              }
                           }
                        }
                     }
                  }

                  // check if it's chunked
                  if(CChar8 *t=TextPos(_header, "Transfer-Encoding:"))
                  {
                     Int line=TextPosI(t, "\r\n");
                     if( line>=0)((Char8*)t)[line]=0;
                     if(Contains(t, "chunked"))_flags|=CHUNKED;
                  }

                  // process remaining data
                  if(_size>=0) // known size
                  {
                    _data=Alloc(_size);
                     if(rcvd){CopyFast(_data, rest, rcvd); _done+=rcvd;}
                  }else // unknown size
                  if(chunked()) // chunked
                  {
                     if(rcvd)parse(rest, rcvd);
                  }else // remaining data as long as received
                  {
                    _memb.addNum(rcvd); REPAO(_memb)=rest[i];
                    _done+=rcvd;
                  }
               }
            }else
            {
               if(_size>=0) // known size
               {
                  Long left=_size-_done;
                  if(  left>0)
                     if(_socket.wait(DOWNLOAD_WAIT_TIME))
                  {
                     left=receive((Byte*)_data+_done, Min(INT_MAX, left));
                     if(left>0)_done+=left;else return error();
                  }
                  if(_done>=_size)finish(); // set the state at final stage (to avoid multi-threading issues)
               }else
               if(chunked()) // chunked
               {
                  if(_parse==PARSE_END) // finished
                  {
                    _data=Alloc(_memb.elms()); _memb.copyTo((Byte*)_data);
                    _size=_total_size=_memb.elms();
                    _memb.del();
                     finish(); // set the state at final stage (to avoid multi-threading issues)
                  }else
                  if(_socket.wait(DOWNLOAD_WAIT_TIME))
                  {
                     Int size=receive(data, BUF_SIZE);
                     if( size>0)parse(data, size);else return error();
                  }
               }else
               {
                  if(_socket.wait(DOWNLOAD_WAIT_TIME))
                  {
                     Int rcvd=receive(data, BUF_SIZE);
                     if( rcvd>0)
                     {
                        FREP(rcvd)_memb.add(data[i]);
                       _done+=rcvd;
                     }else
                     {
                       _data=Alloc(_memb.elms()); _memb.copyTo((Byte*)_data);
                       _size=_total_size=_memb.elms();
                       _memb.del();
                        finish(); // set the state at final stage (to avoid multi-threading issues)
                     }
                  }
               }
            }
         }
      }break;

      default: delPartial(); return false;
   }
   return true;
}
#endif
/******************************************************************************/
Str8 Download::fileHeader(Int i)C
{
 C HTTPFile &file=_files[i];
   Str8 h=_file_header;
   if(file.name.is())AppendName(h, file.name);else h+=i; // if name is empty then use index
   h+="\"; filename=\""; // this is required too, without this, server will treat as if no file
   if(file.name.is())AppendName(h, file.name);else h+=i; // if name is empty then use index
   h+="\"\r\n";
   h+=S8+"Content-Length: "+file.Send()+"\r\n";
 //h+=S8+"Last-Modified: Thu, 24 Mar 2011 18:35:38 GMT\r\n"; modify time is always ignored, instead have to adjust modify time on PHP
   h+="\r\n";
   return h;
}
/******************************************************************************/
void Download::zero()
{
  _flags=_parse=0;
  _state=DWNL_NONE;
  _code=0;
  _expected_size=_send_pos=_file_i=0;
  _offset=_done=_size=_total_size=_sent=_to_send=_total_sent=_total_rcvd=0;
  _data=null;
  _event=null;
  _modif_time.zero();
#if WEB
  _js_download=null;
#endif
}

Download::Download(                                                                                                     ) : _memb(64*1024) {zero();                                                  }
Download::Download(C Str &url, C CMemPtr<HTTPParam> &params, MemPtr<HTTPFile> files, Long offset, Long size, Bool paused) : _memb(64*1024) {zero(); create(url, params, files, offset, size, paused);}

void Download::delPartial() // this deletes only members which can't be accessed on the main thread
{
  _socket     .del  ();
  _memb       .del  ();
  _addrs      .clear();
  _url_full   .clear();
  _send       .clear();
  _footer     .clear();
  _header     .clear();
  _file_header.clear();
}
Download& Download::del(Int milliseconds)
{
#if WEB
   if(_js_download)
   {
      SyncLocker lock(JSDownloadsLock);
      if(JSDownloads.elms()) // check in case 'JSDownloads' destructor was already called
         if(JSDownload *js_download=(JSDownload*)_js_download)js_download->download=null; // unlink, however don't remove it, because this will be done through the JS (since JS still holds pointer to this)
   }
#endif
   stop();
  _thread.del(milliseconds);
   delPartial();
  _files.point(null);
  _url.clear();
   Free(_data);
   if(state())state(DWNL_NONE); // call here because 'zero' does not call this
   zero(); return T;
}
static CChar8 BoundaryChars[]= // without space
{
   '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
   'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
   'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
   '(', ')', '+', '_', ',', '-', '.', '/', ':', '=', '?', '\'',
};
/* https://www.rfc-editor.org/rfc/rfc2046#section-5.1.1

boundary := 0*69<bchars> bcharsnospace
bchars := bcharsnospace / " "
bcharsnospace := DIGIT / ALPHA / "'" / "(" / ")" / "+" / "_" / "," / "-" / "." / "/" / ":" / "=" / "?"
*/
Download& Download::create(C Str &url, C CMemPtr<HTTPParam> &params, MemPtr<HTTPFile> files, Long offset, Long size, Bool paused, Bool ignore_auth_result, SyncEvent *event)
{
   if(!files.elms()
   || WEB)files.point(null); // TODO: sending files on WEB not yet supported

   del();

  _flags     =(ignore_auth_result ? AUTH_IGNORE : 0);
  _url       =url;
  _offset    =offset;
  _size      =size;
  _total_size=-1; // unknown total size
  _files     .point(files);
  _event     =event;
   REPA(files)_to_send+=files[i].Send();

   // once base params are set, set state
   state(DWNL_CONNECTING); // this must be set before returning, so that we mark this 'Download' as used

   // set url full
   Bool url_has_params=AppendUrlPath(_url_full, UTF8(url)), has_post_params=false;
   FREPA(params) // add parameters
   {
    C HTTPParam &param=params[i]; switch(param.type)
      {
         case HTTP_POST:
         case HTTP_POST_BIN:
            has_post_params=true; break;

         case HTTP_GET:
         {
           _url_full+=(url_has_params ? '&' : '?'); // first param must be started with '?', others with '&'
            url_has_params=true;
            AppendParam(_url_full, param);
         }break;
      }
   }

   // set message
   Str8 prefix,
        request=((files || has_post_params) ? "POST" : (_size==0) ? "HEAD" : "GET");
       _send   =GetHeaders(_url_full, request);
   Str8 bytes  =GetRange  (T);
   if(  bytes.is())_send+=S8+"Range: "+bytes+"\r\n";

   FREPA(params) // header params
   {
    C HTTPParam &param=params[i]; if(param.type==HTTP_HEADER)_send+=UTF8(param.name)+": "+UTF8(param.value)+"\r\n";
   }

   if(files)
   {
      // sub header
      const Int boundary_len=70; Str8 boundary; boundary.reserve(boundary_len); REP(boundary_len)boundary+=Random.elm(BoundaryChars); // 'boundary' must not be present in the file data !! otherwise the upload will fail !! up to 70 characters - https://www.rfc-editor.org/rfc/rfc2046#section-5.1

      FREPA(params) // params
      {
       C HTTPParam &param=params[i]; switch(param.type)
         {
            case HTTP_POST:
            case HTTP_POST_BIN:
            {
               prefix+=S8+"--"+boundary+"\r\n";
               prefix+="Content-Disposition: form-data; name=\"";
               AppendName(prefix, param.name);
               prefix+="\"\r\n";
               prefix+="Content-Type: form-data\r\n";
               if(param.type==HTTP_POST)
               {
                C Str8 &value=UTF8(param.value);
                  prefix+=S8+"Content-Length: "+value.length()+"\r\n"; // data length
                  prefix+="\r\n";
                  prefix+=value;
               }else
               {
                C Str &value=param.value;
                  prefix+=S8+"Content-Length: "+value.length()+"\r\n"; // data length
                  prefix+="\r\n";
                  prefix+=value;
               }
               prefix+="\r\n";
            }break;
         }
      }

     _file_header =S8+"--"+boundary+"\r\n";
     _file_header+="Content-Type: application/octet-stream\r\n";
     _file_header+="Content-Transfer-Encoding: binary\r\n";
     _file_header+="Content-Disposition: form-data; name=\"";

      // header/suffix for file #0 is already included in main _send/_footer
      prefix+=fileHeader(0); // file #0 header is here
      // file #0 data is here

      // other files will have it added dynamically in the update loop
         // file suffix is here
         // file header is here
         // file data   is here
         Int start=1, file_headers=(files.elms()-start)*FileSuffixLen();
         for(Int i=start; i<files.elms(); i++)file_headers+=fileHeader(i).length();

     _footer+=FileSuffix(); // file suffix is here
     _footer+=S8+"--"+boundary+"--\r\n";

      // now when 'prefix' and 'suffix' are ready, setup the main header
     _send+=S8+"Content-Type: multipart/form-data; boundary=\""+boundary+"\"\r\n";
     _send+=S8+"Content-Length: "+(prefix.length()+toSend()+file_headers+_footer.length())+"\r\n";
     _send+="Cache-Control: no-cache\r\n";
   }else
   if(has_post_params) // just POST params
   {
      Bool added=false;
      FREPA(params)
      {
       C HTTPParam &param=params[i]; switch(param.type)
         {
            case HTTP_POST    : if(added)prefix+='&'; AppendParam   (prefix, param); added=true; break;
            case HTTP_POST_BIN: if(added)prefix+='&'; AppendParamBin(prefix, param); added=true; break;
         }
      }
     _send+="Content-type: application/x-www-form-urlencoded\r\n";
     _send+="Content-Transfer-Encoding: binary\r\n";
     _send+=S8+"Content-Length: "+prefix.length()+"\r\n";
   }
  _send+="\r\n";
  _send+=prefix;

#if !WEB
   if(!_thread.create(DownloadFunc, this, 0, paused, "EE.Download"))error(); // thread as last
#else
   // if we're running a game from "esenthel.com" then it cannot access files from "www.esenthel.com" and vice versa
   // therefore we detect where the game is from, and if the domain matches, but www is different, then we adjust it
   if(_url_full[0]!='/') // ignore following code if we're just accessing the resource from website root like "/site/files/xx"
      if(Bool url_has_http=StartsPath(_url_full, "http://")) // we can do this only if we're accessing using full path (to avoid cases when downloading "www.esenthel.com" where it's actually a folder name on the server)
   {
      if(url_has_http)_url_full=SkipStartPath(_url_full, "http://"); // eat HTTP
      Str8          url_domain  =GetStart(_url_full);
      Str8 document_url_domain  =GetStart(SkipStartPath(JavaScriptRunS("document.URL"), "http://"));
      Bool document_url_domain_has_www=Starts(document_url_domain, "www.");
      Bool          url_domain_has_www=Starts(         url_domain, "www.");
      if(document_url_domain_has_www!=url_domain_has_www) // difference in www
      {
         if(document_url_domain_has_www)document_url_domain=SkipStart(document_url_domain, "www.");
         if(         url_domain_has_www)         url_domain=SkipStart(         url_domain, "www.");
         if(url_domain==document_url_domain) // if the domain is the same, case insensitive
         {
            if(document_url_domain_has_www)_url_full=S8+"www."+_url_full;          // the document has    www, so we need to add    www prefix
            else                           _url_full=SkipStart(_url_full, "www."); // the document has no www, so we need to remove www prefix
         }
      }
      if(url_has_http)_url_full=S8+"http://"+_url_full; // restore HTTP
   }

   JSDownload &js_download=JSDownloads.New();
   js_download.download=this;
  _js_download=&js_download;

   EM_ASM
   ({
      var request=Pointer_stringify($0); var url=Pointer_stringify($1); var range_bytes=Pointer_stringify($2);
      var post_params=Pointer_stringify($3); var user=$4; var onprogress=$5; var onload=$6; var onerror=$7;
      var xhr=new XMLHttpRequest();
      xhr.open(request, url, true);
      xhr.responseType='arraybuffer';
      if(range_bytes!='')xhr.setRequestHeader('Range', range_bytes);

      // progress
      xhr.onprogress=function http_onprogress(e)
      {
         if(user!=null)
         {
            var date=xhr.getResponseHeader('Last-Modified'); date=((date!=null) ? new Date(date).getTime()/1000 : 0);
            Runtime.dynCall('viiiii', onprogress, [user, xhr.status, e.loaded, e.total, date]);
         }
      };

      // done
      xhr.onload=function http_onload(e)
      {
         if(user!=null)
         {
            var content_length=xhr.getResponseHeader('Content-Length'); if(content_length===null)content_length=-1; // needed for HEAD requests
            var content_range =xhr.getResponseHeader('Content-Range'); var cr=-1; if(content_range!=null){var i=content_range.indexOf('/'); if(i>=0)cr=parseInt(content_range.substr(i+1));}
            var date=xhr.getResponseHeader('Last-Modified'); date=((date!=null) ? new Date(date).getTime()/1000 : 0);
            var byteArray=new Uint8Array(xhr.response);
            var buffer=_malloc(byteArray.length);
            HEAPU8.set(byteArray, buffer);
            Runtime.dynCall('viiiiiii', onload, [user, xhr.status, buffer, byteArray.length, content_length, cr, date]);
           _free(buffer);
            user=null;
         }
      };

      // error
      xhr.onerror=function http_onerror(e)
      {
         if(user!=null)
         {
            Runtime.dynCall('vii', onerror, [user, xhr.status]);
            user=null;
         }
      };

      // limit the number of redirections
      try{if(xhr.channel instanceof Ci.nsIHttpChannel)xhr.channel.redirectionLimit=0;}catch(ex){}

      // send
      if(request!="POST")xhr.send(null);else
      {
         xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
         xhr.setRequestHeader("Content-length", post_params.length);
         xhr.setRequestHeader("Connection", "close");
         xhr.send(post_params);
      }
   }, request(), _url_full(), bytes(), prefix(), _js_download, OnProgress, OnDone, OnError);
#endif
   return T;
}
Download& Download::pause (                ) {_thread.pause (            ); return T;}
Download& Download::resume(                ) {_thread.resume(            ); return T;}
Download& Download::stop  (                ) {_thread.stop  (            ); return T;}
Download& Download::wait  (Int milliseconds) {_thread.wait  (milliseconds); return T;}
void      Download::state (DWNL_STATE state) {_state=state; if(_event)_event->on();}
void      Download::finish(                ) {state((code()>=200 && code()<300) ? DWNL_DONE : DWNL_ERROR);}
Bool      Download::error (                ) // !! this is not thread-safe !! this can be called only on the download thread or if the thread is not running
{
   stop();
   delPartial();
   state(DWNL_ERROR); // set this as last
   return false;
}
Bool Download::authFailed()C {return FlagOn(_flags, AUTH_FAILED);}
/******************************************************************************/
}
/******************************************************************************/
