
typedef enum
{
	kIgnoreFile = 0,
	kLockFile,
	kUnlockFile,
	
	kIgnoreName = kIgnoreFile,
	kLockName,
	kUnlockName
};

class NameFileLocker : public boxmaker
{
	public:
		NameFileLocker( int theFileAction, int theNameAction);

	protected:
		virtual void OpenDoc( Boolean opening);
		
	private:
		const int fileAction;
		const int nameAction;
};
