#ifndef _HEADER_SPEECHCONCEPT_INCLUDE_
#define _HEADER_SPEECHCONCEPT_INCLUDE_

#include "extension.h"
#include "utlsymbol.h"
#include "ehandle.h"


namespace MyResponceRules
{
    class CRR_Concept
    {
    public: // local typedefs
        typedef CUtlSymbol tGenericId;
        tGenericId m_iConcept;

    public:
        CRR_Concept() {};
        CRR_Concept(const char *fromString);

        const char *GetStringConcept() const;
        static const char *GetStringForGenericId(tGenericId genericId);

        operator tGenericId() const { return m_iConcept; }
        operator const char *() const { return GetStringConcept(); }
        inline bool operator==(const CRR_Concept &other)
        {
            return m_iConcept == other.m_iConcept;
        }
        bool operator==(const char *pszConcept);

    protected:

    private:
        CRR_Concept& operator=(const char *fromString);
    };
};

class AI_CriteriaSet
{
public:
	AI_CriteriaSet();
	AI_CriteriaSet( const AI_CriteriaSet& src );
	~AI_CriteriaSet();

	void AppendCriteria( const char *criteria, const char *value = "", float weight = 1.0f );
	void RemoveCriteria( const char *criteria );
	
	void Describe();

	int GetCount() const;
	int			FindCriterionIndex( const char *name ) const;

	const char *GetName( int index ) const;
	const char *GetValue( int index ) const;
	float		GetWeight( int index ) const;

private:

	struct CritEntry_t
	{
		CritEntry_t() : criterianame( UTL_INVAL_SYMBOL ), weight( 0.0f )
		{
			value[ 0 ] = 0;
		}

		CritEntry_t( const CritEntry_t& src )
		{
			criterianame = src.criterianame;
			value[ 0 ] = 0;
			weight = src.weight;
			SetValue( src.value );
		}

		CritEntry_t& operator=( const CritEntry_t& src )
		{
			if ( this == &src )
				return *this;

			criterianame = src.criterianame;
			weight = src.weight;
			SetValue( src.value );

			return *this;
		}

		static bool LessFunc( const CritEntry_t& lhs, const CritEntry_t& rhs )
		{
			return Q_stricmp( lhs.criterianame.String(), rhs.criterianame.String() ) < 0 ? true : false;
		}

		void SetValue( char const *str )
		{
			if ( !str )
			{
				value[ 0 ] = 0;
			}
			else
			{
				Q_strncpy( value, str, sizeof( value ) );
			}
		}

		CUtlSymbol	criterianame;
		char		value[ 64 ];
		float		weight;
	};

	CUtlRBTree< CritEntry_t, short > m_Lookup;
};

class CAI_Concept : public MyResponceRules::CRR_Concept
{
public:
    CAI_Concept() {};
    CAI_Concept(const char *fromString) : CRR_Concept(fromString) {} ;
    inline CHandle<CBaseEntity> GetSpeaker() const { return m_hSpeaker; }
    inline void SetSpeaker(CHandle<CBaseEntity> val) { m_hSpeaker = val; }
protected:
    CHandle<CBaseEntity> m_hSpeaker;
};

#endif
