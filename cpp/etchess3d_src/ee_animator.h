#ifndef _EE_ANIMATOR_H_
#define _EE_ANIMATOR_H_

template <class A_Type>
class EE_Animator
{
	A_Type initial_value;
	A_Type current_value;
	A_Type (*animate_func)(float deltatime, void *parent_animator);
public:
	bool ended;
	vector<A_Type> *control_points;
	vector<float> *keyframes;
	float current_time;

	void update(float deltatime);
	EE_Animator( A_Type inital, A_Type (*anim_func)(float, void *));
	~EE_Animator();
	void set_control_points(vector<A_Type> *ctrlpts, vector<float> *keyframes);
	A_Type get_value();
	void clear();
	bool stopped() {return this->ended;}
};

template <class A_Type>
void EE_Animator<A_Type>::clear()
{
	current_value = initial_value;
	if ( NULL != keyframes ) free( keyframes );
	if ( NULL != control_points) free( control_points );
	keyframes = NULL;
	control_points = NULL;
	current_time = 0.0f;
	ended = false;
}

template <class A_Type>
EE_Animator<A_Type>::~EE_Animator()
{
	free( control_points );
	free( keyframes );
}

template <class A_Type> 
EE_Animator<A_Type>::EE_Animator( A_Type initial, A_Type (*anim_func)(float, void *) )
{
	initial_value = initial;
	current_value = initial;
	animate_func = anim_func;
	current_time = 0.0f;
	control_points = NULL;
	keyframes = NULL;
	ended = false;
}

template <class A_Type> 
void EE_Animator<A_Type>::update(float deltatime)
{
	current_time += deltatime;
	current_value = animate_func(deltatime, this);
}

template <class A_Type> 
void EE_Animator<A_Type>::set_control_points(vector<A_Type> *ctrlpts, vector<float> *keyframepts)
{
	free( control_points );
	free( keyframes );
	control_points = new vector<A_Type>();
	keyframes = new vector<float>();
	for( int i = 0; i < ctrlpts->size(); i++)
	{
		control_points->push_back( (*ctrlpts)[i] );
	}
	for( int i = 0; i < keyframepts->size(); i++)
	{
		keyframes->push_back( (*keyframepts)[i] );
	}
}

template <class A_Type>
A_Type EE_Animator<A_Type>::get_value()
{
	return current_value;
}

#endif /* _EE_ANIMATOR_H_*/