
class eight_to_one : public boxmaker
{
	public:
		eight_to_one::eight_to_one();

	protected:
		virtual void OpenDoc( Boolean opening);
		virtual void DoMenu( long retVal);
		virtual Boolean mayEnterFolder( Boolean opening);
		virtual void EnterFolder( Boolean opening);
		virtual void ExitFolder( Boolean opening);
		
	private:
		int folder_depth;
};

inline eight_to_one::eight_to_one() : boxmaker( 3000)
{
	folder_depth = 0;
}

inline Boolean eight_to_one::mayEnterFolder( Boolean opening)
{
	return (folder_depth == 0);
}

inline void eight_to_one::EnterFolder( Boolean opening)
{
	folder_depth += 1;
}

inline void eight_to_one::ExitFolder( Boolean opening)
{
	folder_depth -= 1;
}
