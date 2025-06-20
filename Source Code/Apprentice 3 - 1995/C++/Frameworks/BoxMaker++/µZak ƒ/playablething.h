//
// playablething is an abstract class implementing any
//
class playablething
{
	public:
		virtual void idleTime() {};
		virtual void SetVolume( short newVolume) {};
		virtual void Start() = 0;
		virtual void Stop() = 0;
		virtual Boolean isDone() const = 0;
};

class movieplayer : public playablething
{
	public:
		movieplayer( FSSpec theFSSpec);
		~movieplayer();

		virtual void idleTime() {};
		virtual void SetVolume( short newVolume) {};
		virtual void Start() = 0;
		virtual void Stop() = 0;
		virtual Boolean isDone() const = 0;

	private:
		Movie theMovie;
};

class MODplayer : public playablething
{
	public:
		MODplayer( FSSpec theFSSpec);
		~MODplayer();

		virtual void idleTime() {};
		virtual void SetVolume( short newVolume) {};
		virtual void Start() = 0;
		virtual void Stop() = 0;
		virtual Boolean isDone() const = 0;

	private:
};

class AIFFplayer : public playablething
{
	public:
		AIFFplayer( FSSpec theFSSpec);
		~AIFFplayer();

		virtual void idleTime() {};
		virtual void SetVolume( short newVolume) {};
		virtual void Start() = 0;
		virtual void Stop() = 0;
		virtual Boolean isDone() const = 0;
		
	private:
};
