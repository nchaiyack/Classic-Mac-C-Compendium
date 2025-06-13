
class typicalshell : public boxmaker
{
	public:
		typicalshell();

	protected:
		virtual void OpenDoc( Boolean opening);

		virtual void DoMenu( long retVal);
		
	private:
		static const int kOurQuitItem;
		static const OSType dontChange;

		OSType creatorToSet;
		OSType typeToSet;
};
