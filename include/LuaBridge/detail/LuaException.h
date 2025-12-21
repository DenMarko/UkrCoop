// https://github.com/vinniefalco/LuaBridge
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2008, Nigel Atkinson <suprapilot+LuaCode@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef _INCLUDE_SOURCEMOD_LUAEXEPTION_PROPER_H_
#define _INCLUDE_SOURCEMOD_LUAEXEPTION_PROPER_H_

#include "CString.h"
#include "log_messege.h"

namespace luabridge {

class LuaException
{
private:
    lua_State* m_L;
    String m_what;

public:
    //----------------------------------------------------------------------------
    /**
        Construct a LuaException after a lua_pcall().
    */
    LuaException(lua_State* L, int /*code*/) : m_L(L) { whatFromStack(); }

    //----------------------------------------------------------------------------

    LuaException(lua_State* L, char const*, char const*, long) : m_L(L) { whatFromStack(); }

    //----------------------------------------------------------------------------

    ~LuaException() {}

    //----------------------------------------------------------------------------

    char const* what() const { return m_what.c_str(); }

    //============================================================================
    /**
        Throw an exception.

        This centralizes all the exceptions thrown, so that we can set
        breakpoints before the stack is unwound, or otherwise customize the
        behavior.
    */
    template<class Exception>
    static void Throw(Exception e)
    {
        m_sLog->LogToFileEx(false, "[LUA] %s", e.what());
    }

    //----------------------------------------------------------------------------
    /**
        Wrapper for lua_pcall that throws.
    */
    static void pcall(lua_State* L, int nargs = 0, int nresults = 0, int msgh = 0)
    {
        int code = lua_pcall(L, nargs, nresults, msgh);

        if (code != LUABRIDGE_LUA_OK)
            Throw(LuaException(L, code));
    }

    //----------------------------------------------------------------------------
    /**
        Initializes error handling. Subsequent Lua errors are translated to C++ exceptions.
    */
    static void enableExceptions(lua_State* L) { lua_atpanic(L, throwAtPanic); }

    /** Retrieve the lua_State associated with the exception.

      @returns A Lua state.
    */
    lua_State* state() const { return m_L; }

protected:
    void whatFromStack()
    {
        if (lua_gettop(m_L) > 0)
        {
            char const* s = lua_tostring(m_L, -1);
            m_what = s ? s : "";
        }
        else
        {
            // stack is empty
            m_what = "missing error";
        }
        m_sLog->LogToFileEx(false, "[LUA] %s",  m_what.c_str());
    }

private:
    static int throwAtPanic(lua_State* L)
    {
        LuaException(L, -1);
        return 1;
    }
};

//----------------------------------------------------------------------------
/**
    Initializes error handling. Subsequent Lua errors are translated to C++ exceptions.
*/
static void enableExceptions(lua_State* L)
{
    LuaException::enableExceptions(L);
}

} // namespace luabridge
#endif
