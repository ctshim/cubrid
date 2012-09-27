/*
 * Copyright (C) 2008 Search Solution Corporation. All rights reserved by Search Solution.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */
#include "DBGWCommon.h"
#include "DBGWError.h"
#include "DBGWPorting.h"
#include "DBGWValue.h"
#include "DBGWLogger.h"
#include "DBGWQuery.h"
#include "DBGWDataBaseInterface.h"
#include "DBGWConfiguration.h"
#include "DBGWClient.h"
#include "DBGWAdapter.h"

namespace dbgw
{

  namespace adapter
  {

    namespace DBGW
    {

#define CONVERT_PREVIOUS_DBGWEXCEPTION(e, err) \
  do { \
      convertPreviousDBGWException(e, err, #err); \
  } while (0)

      void convertPreviousDBGWException(const DBGWException &e,
          int nDefaultErrorCode, const char *szDefaultErrorMessage)
      {
        switch (e.getErrorCode())
          {
          case DBGWErrorCode::EXTERNAL_DBGW_INVALID_HANDLE:
            nDefaultErrorCode = DBGWCONNECTOR_INVALID_HANDLE;
            szDefaultErrorMessage = "DBGWCONNECTOR_INVALID_HANDLE";
            break;
          case DBGWErrorCode::EXTERNAL_MEMORY_ALLOC_FAIL:
            nDefaultErrorCode = DBGWCONNECTOR_NOT_ENOUGH_MEMORY;
            szDefaultErrorMessage = "DBGWCONNECTOR_NOT_ENOUGH_MEMORY";
            break;
          case DBGWErrorCode::RESULT_NOT_ALLOWED_OPERATION:
            nDefaultErrorCode = DBGWCONNECTOR_NOT_PROPER_OP;
            szDefaultErrorMessage = "DBGWCONNECTOR_NOT_PROPER_OP";
            break;
          case DBGWErrorCode::RESULT_NO_MORE_DATA:
            nDefaultErrorCode = DBGWCONNECTOR_NOMORE_FETCH;
            szDefaultErrorMessage = "DBGWCONNECTOR_NOMORE_FETCH";
            break;
          case DBGWErrorCode::CLIENT_ALREADY_IN_TRANSACTION:
            nDefaultErrorCode = DBGWCONNECTOR_ALREAY_IN_TRANSACTION;
            szDefaultErrorMessage = "DBGWCONNECTOR_ALREAY_IN_TRANSACTION";
            break;
          case DBGWErrorCode::CLIENT_NOT_IN_TRANSACTION:
            nDefaultErrorCode = DBGWCONNECTOR_NOT_IN_TRANSACTION;
            szDefaultErrorMessage = "DBGWCONNECTOR_NOT_IN_TRANSACTION";
            break;
          }

        DBGWPreviousException pe = DBGWPreviousExceptionFactory::create(e,
            nDefaultErrorCode, szDefaultErrorMessage);
        DBGW_LOG_ERROR(pe.what());
        setLastException(pe);
      }

      DECLSPECIFIER unsigned int __stdcall GetLastError()
      {
        return getLastErrorCode();
      }

      namespace Environment
      {

        DECLSPECIFIER Handle __stdcall CreateHandle(const char *szConfFileName)
        {
          clearException();

          Handle hEnv = NULL;
          try
            {
              hEnv = new DBGWConfiguration(szConfFileName);
              if (hEnv == NULL)
                {
                  MemoryAllocationFail e(sizeof(Handle));
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              if (getLastErrorCode() != DBGWErrorCode::NO_ERROR)
                {
                  throw getLastException();
                }

              return hEnv;
            }
          catch (DBGWException &e)
            {
              if (hEnv != NULL)
                {
                  delete hEnv;
                }

              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_CREATE_FAILED);
              return NULL;
            }
        }

        DECLSPECIFIER void __stdcall DestroyHandle(Handle hEnv)
        {
          clearException();

          try
            {
              if (hEnv == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              delete hEnv;
            }
          catch (DBGWException &e)
            {
              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_INVALID_PARAMETER);
            }
        }

        DECLSPECIFIER bool __stdcall LoadConnector(Handle hEnv,
            const char *szXmlPath)
        {
          clearException();

          try
            {
              if (hEnv == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              return hEnv->loadConnector(szXmlPath);
            }
          catch (DBGWException &e)
            {
              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_INVALID_PARAMETER);
              return false;
            }
        }

        DECLSPECIFIER bool __stdcall LoadQueryMapper(Handle hEnv,
            const char *szXmlPath, bool bAppend)
        {
          clearException();

          try
            {
              if (hEnv == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              return hEnv->loadQueryMapper(szXmlPath, bAppend);
            }
          catch (DBGWException &e)
            {
              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_INVALID_PARAMETER);
              return false;
            }
        }

      }

      namespace Connector
      {

        DECLSPECIFIER Handle __stdcall CreateHandle(Environment::Handle hEnv)
        {
          clearException();

          Handle hConnector = NULL;
          try
            {
              if (hEnv == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              hConnector = new Environment::Handle();
              if (hConnector == NULL)
                {
                  MemoryAllocationFail e(sizeof(Environment::Handle));
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              *hConnector = hEnv;

              return hConnector;
            }
          catch (DBGWException &e)
            {
              if (hConnector != NULL)
                {
                  delete hConnector;
                }

              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_CREATE_FAILED);
              return NULL;
            }
        }

        DECLSPECIFIER void __stdcall DestroyHandle(Handle hConnector)
        {
          clearException();

          try
            {
              if (hConnector == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              delete hConnector;
            }
          catch (DBGWException &e)
            {
              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_INVALID_PARAMETER);
            }
        }

      }

      namespace ParamSet
      {

        DECLSPECIFIER Handle __stdcall CreateHandle()
        {
          clearException();

          Handle hParam = NULL;
          try
            {
              hParam = new DBGWParameter();
              if (hParam == NULL)
                {
                  MemoryAllocationFail e(sizeof(Handle));
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              if (getLastErrorCode() != DBGWErrorCode::NO_ERROR)
                {
                  throw getLastException();
                }

              return hParam;
            }
          catch (DBGWException &e)
            {
              if (hParam != NULL)
                {
                  delete hParam;
                }

              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_CREATE_FAILED);
              return NULL;
            }
        }

        DECLSPECIFIER void __stdcall DestroyHandle(Handle hParam)
        {
          clearException();

          try
            {
              if (hParam == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              delete hParam;
            }
          catch (DBGWException &e)
            {
              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_INVALID_HANDLE);
            }
        }

        DECLSPECIFIER bool __stdcall Clear(Handle hParam)
        {
          clearException();

          try
            {
              if (hParam == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              hParam->clear();
              return true;
            }
          catch (DBGWException &e)
            {
              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_INVALID_HANDLE);
              return false;
            }
        }

        DECLSPECIFIER size_t __stdcall Size(Handle hParam)
        {
          clearException();

          try
            {
              if (hParam == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              return hParam->size();
            }
          catch (DBGWException &e)
            {
              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_INVALID_HANDLE);
              return 0;
            }
        }

        DECLSPECIFIER  bool __stdcall SetParameter(Handle hParam,
            const char *szParamName, int nParamValue)
        {
          clearException();

          try
            {
              if (hParam == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              if (hParam->put(szParamName, nParamValue) == false)
                {
                  throw getLastException();
                }

              return true;
            }
          catch (DBGWException &e)
            {
              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_INVALID_PARAMETER);
              return false;
            }
        }

        DECLSPECIFIER bool __stdcall SetParameter(Handle hParam, int nIndex,
            int nParamValue)
        {
          clearException();

          try
            {
              if (hParam == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              if (hParam->set(nIndex, nParamValue) == false)
                {
                  throw getLastException();
                }

              return true;
            }
          catch (DBGWException &e)
            {
              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_INVALID_PARAMETER);
              return false;
            }
        }

        DECLSPECIFIER bool __stdcall SetParameter(Handle hParam,
            const char *szParamName, int64 nParamValue)
        {
          clearException();

          try
            {
              if (hParam == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              if (hParam->put(szParamName, nParamValue) == false)
                {
                  throw getLastException();
                }

              return true;
            }
          catch (DBGWException &e)
            {
              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_INVALID_PARAMETER);
              return false;
            }
        }

        DECLSPECIFIER bool __stdcall SetParameter(Handle hParam, int nIndex,
            int64 nParamValue)
        {
          clearException();

          try
            {
              if (hParam == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              if (hParam->set(nIndex, nParamValue) == false)
                {
                  throw getLastException();
                }

              return true;
            }
          catch (DBGWException &e)
            {
              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_INVALID_PARAMETER);
              return false;
            }
        }

        DECLSPECIFIER bool __stdcall SetParameter(Handle hParam,
            const char *szParamName, const char *szParamValue, size_t nLen)
        {
          clearException();

          try
            {
              if (hParam == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              if (hParam->put(szParamName, DBGW_VAL_TYPE_STRING,
                  (void *) szParamValue, szParamValue == NULL, nLen) == false)
                {
                  throw getLastException();
                }

              return true;
            }
          catch (DBGWException &e)
            {
              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_INVALID_PARAMETER);
              return false;
            }
        }

        DECLSPECIFIER bool __stdcall SetParameter(Handle hParam, int nIndex,
            const char *szParamValue, size_t nLen)
        {
          clearException();

          try
            {
              if (hParam == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              if (hParam->set(nIndex, DBGW_VAL_TYPE_STRING,
                  (void *) szParamValue, szParamValue == NULL, nLen) == false)
                {
                  throw getLastException();
                }

              return true;
            }
          catch (DBGWException &e)
            {
              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_INVALID_PARAMETER);
              return false;
            }
        }

      }

      namespace ResultSet
      {
        DECLSPECIFIER Handle __stdcall CreateHandle()
        {
          clearException();

          Handle hResult = NULL;
          try
            {
              hResult = new db::DBGWResultSharedPtr();
              if (hResult == NULL)
                {
                  MemoryAllocationFail e(sizeof(Handle));
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              if (getLastErrorCode() != DBGWErrorCode::NO_ERROR)
                {
                  throw getLastException();
                }

              return hResult;
            }
          catch (DBGWException &e)
            {
              if (hResult != NULL)
                {
                  delete hResult;
                }

              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_CREATE_FAILED);
              return NULL;
            }
        }

        DECLSPECIFIER void  __stdcall DestroyHandle(Handle hResult)
        {
          clearException();

          try
            {
              if (hResult == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              delete hResult;
            }
          catch (DBGWException &e)
            {
              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_INVALID_HANDLE);
            }
        }

        DECLSPECIFIER size_t __stdcall GetRowCount(Handle hResult)
        {
          clearException();

          try
            {
              if (hResult == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              size_t nRowCount = (*hResult)->getRowCount();
              if (getLastErrorCode() != DBGWErrorCode::NO_ERROR)
                {
                  throw getLastException();
                }

              return nRowCount;
            }
          catch (DBGWException &e)
            {
              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_NOT_PROPER_OP);
              return 0;
            }
        }

        DECLSPECIFIER bool  __stdcall First(Handle hResult)
        {
          clearException();

          try
            {
              if (hResult == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              if ((*hResult)->first() == false)
                {
                  throw getLastException();
                }

              return true;
            }
          catch (DBGWException &e)
            {
              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_NOT_PROPER_OP);
              return false;
            }
        }

        DECLSPECIFIER bool  __stdcall Fetch(Handle hResult)
        {
          clearException();

          try
            {
              if (hResult == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              if ((*hResult)->next() == false)
                {
                  throw getLastException();
                }

              return true;
            }
          catch (DBGWException &e)
            {
              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_NOT_PROPER_OP);
              return false;
            }
        }

        DECLSPECIFIER size_t __stdcall GetAffectedCount(Handle hResult)
        {
          clearException();

          try
            {
              if (hResult == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              size_t nAffectedRow = (*hResult)->getAffectedRow();
              if (getLastErrorCode() != DBGWErrorCode::NO_ERROR)
                {
                  throw getLastException();
                }

              return nAffectedRow;
            }
          catch (DBGWException &e)
            {
              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_NOT_PROPER_OP);
              return 0;
            }
        }

        DECLSPECIFIER bool __stdcall IsNeedFetch(Handle hResult)
        {
          clearException();

          try
            {
              if (hResult == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              return (*hResult)->isNeedFetch();
            }
          catch (DBGWException &e)
            {
              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_INVALID_HANDLE);
              return 0;
            }
        }

        DECLSPECIFIER const MetaDataList *__stdcall GetMetaDataList(Handle hResult)
        {
          clearException();

          try
            {
              if (hResult == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              const MetaDataList *metaList = (*hResult)->getMetaDataList();
              if (metaList == NULL)
                {
                  throw getLastException();
                }

              return metaList;
            }
          catch (DBGWException &e)
            {
              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_NOT_PROPER_OP);
              return NULL;
            }
        }

        DECLSPECIFIER bool __stdcall GetColumn(Handle hResult, int nIndex,
            int *pValue)
        {
          clearException();

          try
            {
              if (hResult == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              if ((*hResult)->getInt(nIndex, pValue) == false)
                {
                  throw getLastException();
                }

              return true;
            }
          catch (DBGWException &e)
            {
              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_INVALID_PARAMETER);
              return false;
            }
        }

        DECLSPECIFIER bool __stdcall GetColumn(Handle hResult, const char *szName,
            int *pValue)
        {
          clearException();

          try
            {
              if (hResult == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              if ((*hResult)->getInt(szName, pValue) == false)
                {
                  throw getLastException();
                }

              return true;
            }
          catch (DBGWException &e)
            {
              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_INVALID_PARAMETER);
              return false;
            }
        }

        DECLSPECIFIER bool __stdcall GetColumn(Handle hResult, int nIndex,
            int64 *pValue)
        {
          clearException();

          try
            {
              if (hResult == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              if ((*hResult)->getLong(nIndex, pValue) == false)
                {
                  throw getLastException();
                }

              return true;
            }
          catch (DBGWException &e)
            {
              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_INVALID_PARAMETER);
              return false;
            }
        }

        DECLSPECIFIER bool __stdcall GetColumn(Handle hResult, const char *szName,
            int64 *pValue)
        {
          clearException();

          try
            {
              if (hResult == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              if ((*hResult)->getLong(szName, pValue) == false)
                {
                  throw getLastException();
                }

              return true;
            }
          catch (DBGWException &e)
            {
              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_INVALID_PARAMETER);
              return false;
            }
        }

        DECLSPECIFIER bool __stdcall GetColumn(Handle hResult, int nIndex,
            char *szBuffer, int BufferSize, size_t *pLen)
        {
          clearException();

          try
            {
              if (hResult == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              const DBGWValue *pValue = (*hResult)->getValue(nIndex);
              int nValueSize = pValue->size();

              if (pValue->isNull())
                {
                  memset(szBuffer, 0, BufferSize);
                }
              else
                {
                  if (nValueSize > BufferSize)
                    {
                      NotEnoughBufferException e;
                      DBGW_LOG_ERROR(e.what());
                      throw e;
                    }

                  char *szValue = NULL;
                  pValue->getCString(&szValue);

                  memcpy(szBuffer, szValue, nValueSize);
                }

              *pLen = nValueSize;
              return true;
            }
          catch (DBGWException &e)
            {
              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_INVALID_PARAMETER);
              return false;
            }
        }

        DECLSPECIFIER bool __stdcall GetColumn(Handle hResult, const char *szName,
            char *szBuffer, int BufferSize, size_t *pLen)
        {
          clearException();

          try
            {
              if (hResult == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              const DBGWValue *pValue = (*hResult)->getValue(szName);
              int nValueSize = pValue->size();

              if (pValue->isNull())
                {
                  memset(szBuffer, 0, BufferSize);
                }
              else
                {
                  if (nValueSize > BufferSize)
                    {
                      NotEnoughBufferException e;
                      DBGW_LOG_ERROR(e.what());
                      throw e;
                    }

                  char *szValue = NULL;
                  pValue->getCString(&szValue);

                  memcpy(szBuffer, szValue, nValueSize);
                }

              *pLen = nValueSize;
              return true;
            }
          catch (DBGWException &e)
            {
              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_INVALID_PARAMETER);
              return false;
            }
        }

      }

      namespace Executor
      {

        DECLSPECIFIER Handle __stdcall CreateHandle(
            DBGW::Connector::Handle hConnector, const char *szNamespace)
        {
          clearException();

          Handle hExecutor = NULL;
          try
            {
              Environment::Handle hEnv = *hConnector;

              hExecutor = new DBGWClient(*hEnv, szNamespace);
              if (hExecutor == NULL)
                {
                  MemoryAllocationFail e(sizeof(Handle));
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              if (getLastErrorCode() != DBGWErrorCode::NO_ERROR)
                {
                  throw getLastException();
                }

              return hExecutor;
            }
          catch (DBGWException &e)
            {
              if (hExecutor != NULL)
                {
                  delete hExecutor;
                }

              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_CREATE_FAILED);
              return NULL;
            }
        }

        DECLSPECIFIER void __stdcall DestroyHandle(Handle hExecutor)
        {
          clearException();

          try
            {
              if (hExecutor == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              delete hExecutor;
            }
          catch (DBGWException &e)
            {
              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_INVALID_HANDLE);
            }
        }

        DECLSPECIFIER bool __stdcall Execute(Handle hExecutor, const char *szMethod,
            DBGW::ParamSet::Handle hParam, DBGW::ResultSet::Handle &hResult)
        {
          clearException();

          try
            {
              if (hExecutor == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              *hResult = hExecutor->exec(szMethod, hParam);
              if (*hResult == NULL)
                {
                  throw getLastException();
                }

              return true;
            }
          catch (DBGWException &e)
            {
              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_EXEC_FAILED);
              return false;
            }
        }

        DECLSPECIFIER bool __stdcall BeginTransaction(Handle hExecutor)
        {
          clearException();

          try
            {
              if (hExecutor == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              return hExecutor->setAutocommit(false);
            }
          catch (DBGWException &e)
            {
              CONVERT_PREVIOUS_DBGWEXCEPTION(e, DBGWCONNECTOR_EXEC_FAILED);
              return false;
            }
        }

        DECLSPECIFIER bool __stdcall CommitTransaction(Handle hExecutor)
        {
          clearException();

          try
            {
              if (hExecutor == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              return hExecutor->commit();
            }
          catch (DBGWException &e)
            {
              setLastException(e);
              return false;
            }
        }

        DECLSPECIFIER bool __stdcall RollbackTransaction(Handle hExecutor)
        {
          clearException();

          try
            {
              if (hExecutor == NULL)
                {
                  InvalidHandleException e;
                  DBGW_LOG_ERROR(e.what());
                  throw e;
                }

              return hExecutor->rollback();
            }
          catch (DBGWException &e)
            {
              setLastException(e);
              return false;
            }
        }

      }

    }

    DBGWPreviousException::DBGWPreviousException(
        const DBGWExceptionContext &context) throw() :
      DBGWException(context)
    {
    }

    DBGWPreviousException::~DBGWPreviousException() throw()
    {
    }

    DBGWPreviousException DBGWPreviousExceptionFactory::create(
        const DBGWException &e, int nPreviousErrorCdoe,
        const char *szPreviousErrorMessage)
    {
      DBGWExceptionContext context =
      {
        nPreviousErrorCdoe, DBGWErrorCode::NO_ERROR,
        szPreviousErrorMessage, "", false
      };

      stringstream buffer;
      buffer << "[" << context.nErrorCode << "]";
      buffer << " " << szPreviousErrorMessage;
      buffer << " (" << e.what() << ")";
      context.what = buffer.str();

      return DBGWPreviousException(context);
    }

  }

}
