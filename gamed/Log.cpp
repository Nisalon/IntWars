/*
IntWars playground server for League of Legends protocol testing
Copyright (C) 2012  Intline9 <Intline9@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "stdafx.h"
#include "Log.h"
Log* Log::m_instance;

Log::Log()
{
	m_stream = &cout;
	m_type = LT_NORMAL;
	m_instance = NULL;
}

Log::~Log()
{
	if(m_stream != &cout) delete m_stream;
}

void Log::setStream(ostream* stream)
{
	if(m_stream != &cout) delete m_stream;
	m_stream = stream;
}

void Log::setOutputFile(const char* path)
{
	ofstream *filestream = new ofstream();
	filestream->open(path);
	setStream(filestream);
}

void Log::writeDate()
{
	GetSystemTime(&m_time);	
		_snprintf_s(m_buffer,LOG_BUFFER_SIZE,"[%02d/%02d/%02d %02d:%02d:%02d:%03d]",m_time.wDay,
			m_time.wMonth,
			m_time.wYear,
			m_time.wHour,
			m_time.wMinute,
			m_time.wSecond,
			m_time.wMilliseconds);
		 operator<<(*m_stream,m_buffer);
}

void Log::writeLine(const char* buffer,...)
{
	va_list al;
	va_start(al,buffer);	
	vaWriteLine(buffer,al);
	va_end(al);
}

void Log::write(const char* buffer,...)
{
	va_list al;
	va_start(al,buffer);
	vaWrite(buffer,al);
	va_end(al);
}

void Log::errorLine(const char* buffer,...)
{
	va_list al;
	va_start(al,buffer);
	LogType oldtype = m_type;
	m_type = LT_ERROR;
	vaWriteLine(buffer,al);
	m_type = oldtype;
	va_end(al);
}

void Log::error(const char* buffer,...)
{
	va_list al;
	va_start(al,buffer);
	vaWrite(buffer,al);	
	va_end(al);
}

void Log::debugLine(const char* buffer,...)
{
	va_list al;
	va_start(al,buffer);	
	LogType oldtype = m_type;
	m_type = LT_DEBUG;
	vaWriteLine(buffer,al);
	m_type = oldtype;
	va_end(al);
}

void Log::debug(const char* buffer,...)
{
	va_list al;
	va_start(al,buffer);
	vaWrite(buffer,al);	
	va_end(al);
}

void Log::clear()
{
	m_stream->clear();
}

void Log::setLogType(LogType type)
{
	m_type = type;
}

void Log::vaWriteLine(const char* buffer,va_list al)
{
	writeDate();
	switch(m_type)
	{
	case LT_NORMAL:
		operator<<(*m_stream," ");
		break;
	case LT_DEBUG:
		operator<<(*m_stream,"[DEBUG] ");
		break;
	case LT_ERROR:
		operator<<(*m_stream,"[ERROR] ");
		break;
	}
	vsprintf_s(m_buffer,LOG_BUFFER_SIZE,buffer,al);
	operator<<(*m_stream,m_buffer);
}

void Log::vaWrite(const char* buffer,va_list al)
{
	vsprintf_s(m_buffer,LOG_BUFFER_SIZE,buffer,al);
	operator<<(*m_stream,m_buffer);
}

Log* Log::getMainInstance()
{
	if(m_instance == NULL)
	{
		m_instance = new Log();
	}
	return m_instance;
}