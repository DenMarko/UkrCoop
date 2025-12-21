#include "extension.h"
#include "sdk/ai_speechconcept.h"
#include "../Interface/IBaseEntity.h"

unsigned int Sample::my_tolower(unsigned int c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return c + ('a' - 'A');
    }
    else if (c >= 0xFFFFFF90 && c <= 0xFFFFFFAF)
    {
        return c + (0xFFFFFFB0 - 0xFFFFFF90);
    }

    return c;
}

NOINLINE int Sample::my_strncmp(const char *s1, const char *s2, size_t n)
{
    for (size_t i = 0; i < n; i++, s1++, s2++)
    {
        if (my_tolower(*s1) != my_tolower(*s2))
        {
            return (my_tolower(*s1) - my_tolower(*s2));
        }
        if (*s1 == '\0')
        {
            return 0;
        }
    }
    return 0;
}

bool Sample::my_bStrncmp(const char *s1, const char *s2, size_t n)
{
    int nRes = my_strncmp(s1, s2, n);
    if(nRes > 0)
        return false;
    
    if(nRes < 0)
        return false;

    return true;
}

void Sample::FormatName(const char *name, char *res)
{
    while (*name)
    {
        if(*name == '\n' || *name == '\t')
        {
            *res++ = ' ';
        }
        else
        {
            *res++ = *name;
        }
        name++;
    }
    *res = '\0';
}

NOINLINE int Sample::my_strcmp(const char *str1, const char *str2)
{
    while (*str1 && (*str1 == *str2))
    {
        str1++;
        str2++;
    }
    
    return *(const unsigned char*)str1 - *(const unsigned char*)str2;
}

bool Sample::my_bStrcmp(const char *str1, const char *str2)
{
    int nRes = my_strcmp(str1, str2);
    
    if(nRes > 0)
        return false;
    
    if(nRes < 0)
        return false;

    return true;
}

const char *Sample::my_strstr(const char *str, const char *substr)
{
    if (!*substr)
    {
        return ((char *)str);
    }

    char *needle = (char *)substr;
    char *prevloc = (char *)str;
    char *haystack = (char *)str;

    while (*haystack)
    {
        if (my_tolower(*haystack) == my_tolower(*needle))
        {
            haystack++;
            if (!*++needle)
            {
                return prevloc;
            }
        }
        else
        {
            haystack = ++prevloc;
            needle = (char *)substr;
        }
    }

    return NULL;
}

unsigned int Sample::strncopy(char *dest, const char *str, size_t count)
{
    if(!count){
        return 0;
    }

    char *start = dest;
    while((*str) && (--count))
    {
        *dest++ = *str++;
    }
    *dest = '\0';
    return dest - start;
}

char *Sample::my_strcpy(char *destination, const char *source)
{
    char* dest_ptr = destination;
    const char* src_ptr = source;
    while (*src_ptr != '\0')
    {
        *dest_ptr++ = *src_ptr++;
    }
    *dest_ptr = '\0';
    return destination;
}

size_t Sample::UTIL_Format(char *buffer, size_t maxlength, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	size_t len = vsnprintf(buffer, maxlength, fmt, ap);
	va_end(ap);

	if (len >= maxlength)
	{
		buffer[maxlength - 1] = '\0';
		return (maxlength - 1);
	}
	else
	{
		return len;
	}
}

void Sample::UTIL_StringToIntArray(int *pVector, int count, const char *pString)
{
    char *pstr, *pfront, tempString[128];
    int	j;

    Q_strncpy( tempString, pString, sizeof(tempString) );
    pstr = pfront = tempString;

    for ( j = 0; j < count; j++ )
    {
        pVector[j] = atoi( pfront );

        while ( *pstr && *pstr != ' ' )
            pstr++;
        if (!*pstr)
            break;
        pstr++;
        pfront = pstr;
    }

    for ( j++; j < count; j++ )
    {
        pVector[j] = 0;
    }
}

void Sample::UTIL_StringToColor32(color32 *color, const char *pString)
{
    int tmp[4];
    UTIL_StringToIntArray( tmp, 4, pString );
    color->r = tmp[0];
    color->g = tmp[1];
    color->b = tmp[2];
    color->a = tmp[3];
}

void Sample::InsertCommands(const char *str)
{
    char buffer[1024];	
    size_t len = UTIL_Format(buffer, sizeof(buffer) - 2, str);
    buffer[len++] = '\n';
    buffer[len] = '\0';
    engine->InsertServerCommand(buffer);
}

int V_vsnprintfRet( char *pDest, int maxLen, const char *pFormat, va_list params, bool *pbTruncated )
{
	Assert( maxLen > 0 );
	AssertValidWritePtr( pDest, maxLen );
	AssertValidStringPtr( pFormat );

	int len = V_vsnprintf( pDest, maxLen, pFormat, params );

	if ( pbTruncated )
	{
		*pbTruncated = ( len < 0 || len >= maxLen );
	}

	if	( len < 0 || len >= maxLen )
	{
		len = maxLen;
		pDest[maxLen-1] = 0;
	}

	return len;
}

#include "HL2.h"

MyResponceRules::CRR_Concept::CRR_Concept(const char *fromString)
{
    CUtlSymbolTable *g_pRRConceptTable = *(CUtlSymbolTable**)g_HL2->GetAIConceptTable();
    if(g_pRRConceptTable == nullptr)
    {
        g_pRRConceptTable = new CUtlSymbolTable(64, 64, true);
    }

    m_iConcept = (g_pRRConceptTable)->AddString(fromString);
}

bool MyResponceRules::CRR_Concept::operator==(const char *pszConcept)
{
    CUtlSymbolTable **g_pRRConceptTable = (CUtlSymbolTable**)g_HL2->GetAIConceptTable();
	int otherConcept = (*g_pRRConceptTable)->Find(pszConcept);
	return ( otherConcept != UTL_INVAL_SYMBOL && otherConcept == m_iConcept );
}

MyResponceRules::CRR_Concept &MyResponceRules::CRR_Concept::operator=(const char *fromString)
{
    CUtlSymbolTable **g_pRRConceptTable = (CUtlSymbolTable**)g_HL2->GetAIConceptTable();
    if(*g_pRRConceptTable == nullptr)
    {
        *g_pRRConceptTable = new CUtlSymbolTable(64, 64, true);
    }

	m_iConcept = (*g_pRRConceptTable)->AddString(fromString);
	return *this;
}

const char *MyResponceRules::CRR_Concept::GetStringConcept() const
{
    CUtlSymbolTable **g_pRRConceptTable = (CUtlSymbolTable**)g_HL2->GetAIConceptTable();
    if(*g_pRRConceptTable == nullptr)
    {
        *g_pRRConceptTable = new CUtlSymbolTable(64, 64, true);
    }

	const char *retval = (*g_pRRConceptTable)->String(m_iConcept);
	if (retval == NULL)
	{
		Warning( "An RR_Concept couldn't find its string in the symbol table!\n" );
		retval = "";
	}
	return retval;
}

const char *MyResponceRules::CRR_Concept::GetStringForGenericId(tGenericId genericId)
{
    CUtlSymbolTable **g_pRRConceptTable = (CUtlSymbolTable**)g_HL2->GetAIConceptTable();
    if(*g_pRRConceptTable == nullptr)
    {
        *g_pRRConceptTable = new CUtlSymbolTable(64, 64, true);
    }
    return (*g_pRRConceptTable)->String(genericId);
}

AI_CriteriaSet::AI_CriteriaSet() : m_Lookup( 0, 0, CritEntry_t::LessFunc )
{
}

AI_CriteriaSet::AI_CriteriaSet( const AI_CriteriaSet& src ) : m_Lookup( 0, 0, CritEntry_t::LessFunc )
{
	m_Lookup.Purge();
	for ( short i = src.m_Lookup.FirstInorder(); 
		i != src.m_Lookup.InvalidIndex(); 
		i = src.m_Lookup.NextInorder( i ) )
	{
		m_Lookup.Insert( src.m_Lookup[ i ] );
	}
}

AI_CriteriaSet::~AI_CriteriaSet()
{
}

void AI_CriteriaSet::AppendCriteria( const char *criteria, const char *value /*= ""*/, float weight /*= 1.0f*/ )
{
	int idx = FindCriterionIndex( criteria );
	if ( idx == -1 )
	{
		CritEntry_t entry;
		entry.criterianame = criteria;
		MEM_ALLOC_CREDIT();
		entry.SetValue(value);
		entry.weight = weight;
		m_Lookup.Insert( entry );
	}
	else
	{
		CritEntry_t *entry = &m_Lookup[ idx ];
		entry->SetValue( value );
		entry->weight = weight;
	}
}

void AI_CriteriaSet::RemoveCriteria( const char *criteria )
{
	int idx = FindCriterionIndex( criteria );
	if ( idx == -1 )
		return;

	m_Lookup.RemoveAt( idx );
}

int AI_CriteriaSet::GetCount() const
{
	return m_Lookup.Count();
}

int AI_CriteriaSet::FindCriterionIndex( const char *name ) const
{
	CritEntry_t search;
	search.criterianame = name;
	int idx = m_Lookup.Find( search );
	if ( idx == m_Lookup.InvalidIndex() )
		return -1;

	return idx;
}

const char *AI_CriteriaSet::GetName( int index ) const
{
	static char namebuf[ 128 ];
	if ( index < 0 || index >= (int)m_Lookup.Count() )
		return "";

	const CritEntry_t *entry = &m_Lookup[ index ];
	Q_strncpy( namebuf, entry->criterianame.String(), sizeof( namebuf ) );
	return namebuf;
}

const char *AI_CriteriaSet::GetValue( int index ) const
{
	if ( index < 0 || index >= (int)m_Lookup.Count() )
		return "";

	const CritEntry_t *entry = &m_Lookup[ index ];
	return entry->value[0] ? entry->value : "";
}

float AI_CriteriaSet::GetWeight( int index ) const
{
	if ( index < 0 || index >= (int)m_Lookup.Count() )
		return 1.0f;

	const CritEntry_t *entry = &m_Lookup[ index ];
	return entry->weight;
}

void AI_CriteriaSet::Describe()
{
	for ( short i = m_Lookup.FirstInorder(); i != m_Lookup.InvalidIndex(); i = m_Lookup.NextInorder( i ) )
	{
		CritEntry_t *entry = &m_Lookup[ i ];

		if ( entry->weight != 1.0f )
		{
			DevMsg( "  %20s = '%s' (weight %f)\n", entry->criterianame.String(), entry->value[0] ? entry->value : "", entry->weight );
		}
		else
		{
			DevMsg( "  %20s = '%s'\n", entry->criterianame.String(), entry->value[0] ? entry->value : "" );
		}
	}
}

void variant_t::Set( fieldtype_t ftype, void *data )
{
	fieldType = ftype;

	switch ( ftype )
	{
	case FIELD_BOOLEAN:		bVal = *((bool *)data);				break;
	case FIELD_CHARACTER:	iVal = *((char *)data);				break;
	case FIELD_SHORT:		iVal = *((short *)data);			break;
	case FIELD_INTEGER:		iVal = *((int *)data);				break;
	case FIELD_STRING:		iszVal = *((string_t *)data);		break;
	case FIELD_FLOAT:		flVal = *((float *)data);			break;
	case FIELD_COLOR32:		rgbaVal = *((color32 *)data);		break;

	case FIELD_VECTOR:
	case FIELD_POSITION_VECTOR:
	{
		vecVal[0] = ((float *)data)[0];
		vecVal[1] = ((float *)data)[1];
		vecVal[2] = ((float *)data)[2];
		break;
	}

	case FIELD_EHANDLE:		eVal = *((EHANDLE *)data);			break;
	case FIELD_CLASSPTR:	eVal = *((CBaseEntity **)data);		break;
	case FIELD_VOID:		
	default:
		iVal = 0; fieldType = FIELD_VOID;	
		break;
	}
}

void variant_t::SetOther( void *data )
{
	switch ( fieldType )
	{
	case FIELD_BOOLEAN:		*((bool *)data) = bVal != 0;		break;
	case FIELD_CHARACTER:	*((char *)data) = iVal;				break;
	case FIELD_SHORT:		*((short *)data) = iVal;			break;
	case FIELD_INTEGER:		*((int *)data) = iVal;				break;
	case FIELD_STRING:		*((string_t *)data) = iszVal;		break;
	case FIELD_FLOAT:		*((float *)data) = flVal;			break;
	case FIELD_COLOR32:		*((color32 *)data) = rgbaVal;		break;

	case FIELD_VECTOR:
	case FIELD_POSITION_VECTOR:
	{
		((float *)data)[0] = vecVal[0];
		((float *)data)[1] = vecVal[1];
		((float *)data)[2] = vecVal[2];
		break;
	}

	case FIELD_EHANDLE:		*((EHANDLE *)data) = eVal;			break;
	case FIELD_CLASSPTR:	*((CBaseEntity **)data) = eVal;		break;
	default:	break;
	}
}

bool variant_t::Convert( fieldtype_t newType )
{
	if ( newType == fieldType )
	{
		return true;
	}

	//
	// Converting to a null value is easy.
	//
	if ( newType == FIELD_VOID )
	{
		Set( FIELD_VOID, NULL );
		return true;
	}

	//
	// FIELD_INPUT accepts the variant type directly.
	//
	if ( newType == FIELD_INPUT )
	{
		return true;
	}

	switch ( fieldType )
	{
		case FIELD_INTEGER:
		{
			switch ( newType )
			{
				case FIELD_FLOAT:
				{
					SetFloat( (float) iVal );
					return true;
				}

				case FIELD_BOOLEAN:
				{
					SetBool( iVal != 0 );
					return true;
				}

				default:
					break;
			}
			break;

			default:
				break;
		}

		case FIELD_FLOAT:
		{
			switch ( newType )
			{
				case FIELD_INTEGER:
				{
					SetInt( (int) flVal );
					return true;
				}

				case FIELD_BOOLEAN:
				{
					SetBool( flVal != 0 );
					return true;
				}

				default:
					break;
			}
			break;
		}

		//
		// Everyone must convert from FIELD_STRING if possible, since
		// parameter overrides are always passed as strings.
		//
		case FIELD_STRING:
		{
			switch ( newType )
			{
				case FIELD_INTEGER:
				{
					if (iszVal != NULL_STRING)
					{
						SetInt(atoi(STRING(iszVal)));
					}
					else
					{
						SetInt(0);
					}
					return true;
				}

				case FIELD_FLOAT:
				{
					if (iszVal != NULL_STRING)
					{
						SetFloat(atof(STRING(iszVal)));
					}
					else
					{
						SetFloat(0);
					}
					return true;
				}

				case FIELD_BOOLEAN:
				{
					if (iszVal != NULL_STRING)
					{
						SetBool( atoi(STRING(iszVal)) != 0 );
					}
					else
					{
						SetBool(false);
					}
					return true;
				}

				case FIELD_VECTOR:
				{
					Vector tmpVec = vec3_origin;
					if (sscanf(STRING(iszVal), "[%f %f %f]", &tmpVec[0], &tmpVec[1], &tmpVec[2]) == 0)
					{
						// Try sucking out 3 floats with no []s
						sscanf(STRING(iszVal), "%f %f %f", &tmpVec[0], &tmpVec[1], &tmpVec[2]);
					}
					SetVector3D( tmpVec );
					return true;
				}

				case FIELD_COLOR32:
				{
					int nRed = 0;
					int nGreen = 0;
					int nBlue = 0;
					int nAlpha = 255;

					sscanf(STRING(iszVal), "%d %d %d %d", &nRed, &nGreen, &nBlue, &nAlpha);
					SetColor32( nRed, nGreen, nBlue, nAlpha );
					return true;
				}

				case FIELD_EHANDLE:
				{
					// convert the string to an entity by locating it by classname
					CBaseEntity *ent = NULL;
					if ( iszVal != NULL_STRING )
					{
						// FIXME: do we need to pass an activator in here?
                        ent = (CBaseEntity*)g_CallHelper->FindEntityByName(NULL, STRING(iszVal));
						// ent = gEntList.FindEntityByName( NULL, iszVal );
					}
					SetEntity( ent );
					return true;
				}

				default:
					break;
			}
		
			break;
		}

		case FIELD_EHANDLE:
		{
			switch ( newType )
			{
				case FIELD_STRING:
				{
					// take the entities targetname as the string
					string_t iszStr = NULL_STRING;
					if ( eVal != NULL )
					{
						SetString( reinterpret_cast<IBaseEntity*>(eVal.Get())->GetEntityName() );
					}
					return true;
				}

				default:
					break;
			}
			break;
		}
	}

	// invalid conversion
	return false;
}


const char *variant_t::ToString( void ) const
{
	COMPILE_TIME_ASSERT( sizeof(string_t) == sizeof(int) );

	static char szBuf[512];

	switch (fieldType)
	{
	case FIELD_STRING:
		{
			return(STRING(iszVal));
		}

	case FIELD_BOOLEAN:
		{
			if (bVal == 0)
			{
				Q_strncpy(szBuf, "false",sizeof(szBuf));
			}
			else
			{
				Q_strncpy(szBuf, "true",sizeof(szBuf));
			}
			return(szBuf);
		}

	case FIELD_INTEGER:
		{
			Q_snprintf( szBuf, sizeof( szBuf ), "%i", iVal );
			return(szBuf);
		}

	case FIELD_FLOAT:
		{
			Q_snprintf(szBuf,sizeof(szBuf), "%g", flVal);
			return(szBuf);
		}

	case FIELD_COLOR32:
		{
			Q_snprintf(szBuf,sizeof(szBuf), "%d %d %d %d", (int)rgbaVal.r, (int)rgbaVal.g, (int)rgbaVal.b, (int)rgbaVal.a);
			return(szBuf);
		}

	case FIELD_VECTOR:
		{
			Q_snprintf(szBuf,sizeof(szBuf), "[%g %g %g]", (double)vecVal[0], (double)vecVal[1], (double)vecVal[2]);
			return(szBuf);
		}

	case FIELD_VOID:
		{
			szBuf[0] = '\0';
			return(szBuf);
		}

	case FIELD_EHANDLE:
		{
			const char *pszName = (Entity()) ? ((reinterpret_cast<IBaseEntity*>(Entity().Get()))->GetEntityName()).ToCStr() : "<<null entity>>";
			Q_strncpy( szBuf, pszName, 512 );
			return (szBuf);
		}

	default:
		break;
	}

	return("No conversion to string");
}
