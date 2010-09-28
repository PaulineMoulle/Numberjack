 
#include <mistral_sol.h>
#include <iostream>
#include <fstream>
#include <string>


#ifdef _DEBUGSCHED
  #define DBG(fmt, args...) printf("dbg - l%d: "fmt,__LINE__,args)
#else
  #define DBG(fmt, args...)
#endif


namespace MistralScheduler {

  class SchedulingSolver;
  class StatisticList {
  private:
    int best_solution_index;
    int branch_and_bound_index;

  public:

    int    num_nogoods;
    int    num_solutions;
    int    lower_bound;
    int    upper_bound;

    double avg_distance;
    int    min_distance;
    int    max_distance;

    std::vector<long unsigned int> nodes;
    std::vector<long unsigned int> backtracks;
    std::vector<long unsigned int> fails;
    std::vector<long unsigned int> propags;
    std::vector<double>            time;
    std::vector<int>               outcome;

    StatisticList();
    virtual ~StatisticList();

    void add_info(SchedulingSolver *s, const int obj);

    std::ostream& print(std::ostream& os, 
			const char* prefix=" ",
			const int start=0, 
			const int end=-1);
    };


  class ParameterList {
  public:

    static const int LEX     =  0;
    static const int PROMISE =  1;
    static const int ANTI    = -1;
    static const int GUIDED  =  2;
    static const int RGUIDED =  3;
    static const int RAND    =  4;

    static const int nia = 11;
    static const char* int_ident[nia];
    
    static const int nsa = 10;
    static const char* str_ident[nsa];
    
    
    const char* str_param[nsa];
    int int_param[nia];

    char* data_file;
    char* data_file_name;

    int UBinit; // "ub": user defined upper bound (-1 if not)
    int LBinit; // "lb": user defined lower bound (-1 if not)
    int Checked; // "check": whether the solution is checked
    int Seed; // "seed": random seed
    int Cutoff; // "cutoff": time cutoff of dichotomic steps
    int Dichotomy; // "dichotomy": max number of dichotomic steps
    int Base; // "base": base cutoff for restarts
    int Randomized; // "randomized": level of randomization
    int Verbose; // "verbose": level of verbosity
    int Optimise; // "optimise": total time cutoff 
    int Rngd; // "nogood": whether nogood on restart are used

    double Factor;
    double Decay;
    double Skew;

    std::string Policy;
    std::string Heuristic;
    std::string Type; 
    std::string Value;
    std::string DValue;
    std::string IValue;
    std::string Objective;

    int PolicyRestart;

    ParameterList(int length, char** commandline);
    virtual ~ParameterList() {}

    std::ostream& print(std::ostream& os);

  };

  class pair_ {
  public:
    
    int element;
    int rank;

    pair_(const int elt, const int rk) : element(elt), rank(rk) {}
  };

  class Instance {
  private:

    std::vector< std::vector<int> > tasks_in_job;
    std::vector< std::vector<int> > jobs_of_task;
    std::vector< std::vector<int> > tasks_in_machine;
    std::vector< std::vector<int> > machines_of_task;
    std::vector< int >              duration;
    std::vector< int >              release_date;
    std::vector< int >              due_date;

    std::vector< std::vector< pair_ > > task_rank_in_machine;
    std::vector< std::vector< pair_ > > task_rank_in_job;

    int***  setup_time;
    int**  time_lag[2];
    int*   jsp_duedate;
    int*   jsp_latecost;
    int*   jsp_earlycost;

    
    int addTask(const int dur, const int job, const int machine) ;
    void addTaskToJob(const unsigned int index, const unsigned int j) ;
    void addTaskToMachine(const unsigned int index, const unsigned int j) ;


    void osp_readData( const char* filename );
    void sds_readData( const char* filename );
    void jtl_readData( const char* filename );
    void jla_readData( const char* filename );
    void tsp_readData( const char* filename );
    void fsp_readData( const char* filename );
    void jsp_readData( const char* filename );
    void jet_readData( const char* filename );
    void dyn_readData( const char* filename );


    int get_machine_rank(const int ti, const int mj) {
      int rk=-1, mc;
      for(unsigned int i=0; i<task_rank_in_machine[ti].size(); ++i) {
	mc = task_rank_in_machine[ti][i].element;
	if(mc == mj) {
	  rk = task_rank_in_machine[ti][i].rank;
	  break;
	}
      }
      return rk;
    }

    int get_job_rank(const int ti, const int jj) {
      int rk=-1, jb;
      for(unsigned int i=0; i<task_rank_in_job[ti].size(); ++i) {
	jb = task_rank_in_job[ti][i].element;
	if(jb == jj) {
	  rk = task_rank_in_job[ti][i].rank;
	  break;
	}
      }
      return rk;
    }


  public:

    Instance(const ParameterList& params);
    virtual ~Instance();

    std::ostream& print(std::ostream& os);

    int  nJobs() const {return tasks_in_job.size();}
    int  nJobs(const int i) const {return jobs_of_task[i].size();}

    int  nMachines() const {return tasks_in_machine.size();}
    int  nMachines(const int i) const {return machines_of_task[i].size();}

    int  nTasks() const {return duration.size();}
    int  nTasksInJob(const int j) const {return tasks_in_job[j].size();}
    int  nTasksInMachine(const int j) const {return tasks_in_machine[j].size();}

    int  getJobTask(const int i, const int j) const {return tasks_in_job[i][j];}
    int  getMachineTask(const int i, const int j) const {return tasks_in_machine[i][j];}
    int  getLastTaskofJob(const int i) const {return tasks_in_job[i][nTasksInJob(i)-1];}


    int  getJob(const int i, const int j) const {return jobs_of_task[i][j];}
    int  getMachine(const int i, const int j) const {return machines_of_task[i][j];}

    int  getDuration(const int i) const {return duration[i];}
    int  getReleaseDate(const int i) const {return release_date[i];}
    void setReleaseDate(const int i, const int d) {release_date[i] = d;}
    int  getDueDate(const int i) const {return due_date[i];}

    bool hasSetupTime() const {return setup_time != NULL;}
    int  getSetupTime(const int k, const int i, const int j) const;

    bool hasTimeLag() const {return time_lag[0] != NULL;}
    int  getMinLag(const int i, const int j) const {return time_lag[0][i][j];}
    int  getMaxLag(const int i, const int j) const {return time_lag[1][i][j];}

    bool hasJobDueDate() const {return jsp_duedate != NULL;}
    int  getJobDueDate(const int i) const {return jsp_duedate[i];}

    bool hasEarlyCost() const {return jsp_earlycost != NULL; }
    bool hasLateCost() const {return jsp_latecost != NULL; }
    int  getJobEarlyCost(const int i) const {return jsp_earlycost[i];}
    int  getJobLateCost(const int i) const {return jsp_latecost[i];}

    int getMakespanLowerBound();
    int getMakespanUpperBound(const int);

    int getEarlinessTardinessLowerBound(const int);
    int getEarlinessTardinessUpperBound(const int);
  };


  class SchedulingModel : public CSP {
  public:
    
    Instance *data;
    
    int ub_C_max;
    int lb_C_max;

    int ub_L_sum;
    int lb_L_sum;

    VarArray SearchVars;
    
    VarArray tasks;
    VarArray last_tasks;
    VarArray disjuncts;
    VarArray earlybool;
    VarArray latebool;
    Variable C_max;
    Variable L_sum;

    SchedulingModel() : CSP() {}
    SchedulingModel(Instance& prob, const int C_max);
    virtual void setup(Instance& inst, const int C_max);
    virtual ~SchedulingModel();

    virtual int get_lb() = 0;
    virtual int get_ub() = 0;
    virtual void add_objective() = 0;
    virtual int  get_objective() = 0;
    virtual int  set_objective(const int obj) = 0;

  };


  class C_max_Model : public SchedulingModel {
  public:

    C_max_Model() : SchedulingModel() {}
    C_max_Model(Instance& prob, const int C_max) { setup(prob, C_max); }
    virtual ~C_max_Model() {}

    virtual int get_lb();
    virtual int get_ub();    
    virtual void add_objective();
    virtual int  get_objective();
    virtual int  set_objective(const int obj);
  };

  class L_sum_Model : public SchedulingModel {
  public:

    L_sum_Model() : SchedulingModel() {}
    L_sum_Model(Instance& prob, const int L_sum) { setup(prob, L_sum); }
    virtual ~L_sum_Model() {}

    virtual int get_lb();
    virtual int get_ub();    
    virtual void add_objective();
    virtual int  get_objective();
    virtual int  set_objective(const int obj);
  };

  class No_wait_Model : public C_max_Model {
  public:
 
    No_wait_Model() : C_max_Model() {} 
    No_wait_Model(Instance& prob, const int C_max) : C_max_Model() { setup(prob, C_max); }
    virtual void setup(Instance& prob, const int C_max);
    virtual ~No_wait_Model() {}
  };

  class Solution {
  private:
    int *earlybool_value;
    int *latebool_value;
    int *task_min;
    int *task_max;
    int *ltask_value;
    int *disjunct_value;

  public:
    SchedulingModel  * model;
    SchedulingSolver *solver;
    
    Solution(SchedulingModel *m, SchedulingSolver *s);
    virtual ~Solution();

    void guide_search();
    
  };

  
  class SolutionPool {
  public:
    std::vector<Solution*> pool_;
    
    SolutionPool() {}
    virtual ~SolutionPool() {}

    void add(Solution *s) { pool_.push_back(s); }
    Solution* getBestSolution() { return pool_.back(); }
  };


  class SchedulingSolver : public Solver {
  public:

    int lower_bound;
    int upper_bound;
    ParameterList params;
    StatisticList& stats;
    SchedulingModel *model;
    SolutionPool *pool;

    SchedulingSolver(SchedulingModel* m, 
		     ParameterList& p,
		     StatisticList& s) : Solver(*m,m->SearchVars), params(p), stats(s) 
    { 
      model = m; 
      lower_bound = m->get_lb();
      upper_bound = m->get_ub();

      s.lower_bound = lower_bound;
      s.upper_bound = upper_bound;

      pool = new SolutionPool();
      //stats = s; //new StatisticList();

      addHeuristic( params.Heuristic, params.Randomized, params.IValue );
    }
    virtual ~SchedulingSolver() {}


    std::ostream& print_weights(std::ostream& os);
    void decay_weights(const double decay);

    void addHeuristic( std::string Heu, const int rdz, std::string vo ) {
      
      int val_ord = ParameterList::GUIDED;
      if(  vo == "rguided")   val_ord = ParameterList::RGUIDED;
      if(  vo == "lex"    )   val_ord = ParameterList::LEX;
      if(  vo == "rand"   )   val_ord = ParameterList::RAND;
      if(  vo == "promise")   val_ord = ParameterList::PROMISE;
      if(  vo == "anti"   )   val_ord = ParameterList::ANTI;

      if( Heu == "dom" ) {
	MinDomain h(abs(rdz));
	add( h );
      }
      if( Heu == "lex" ) {
	Lexicographic h;
	add( h );
      } 
      else if( Heu == "deg") {
	MaxDegree h(abs(rdz));
	add( h );
      } 
      else if( Heu == "rand") {
	Random h;
	add( h );
      } 
      else if( Heu == "dom+deg") {
	MinDomMaxDeg h(abs(rdz));
	add( h );
      } 
      else if( Heu == "domodeg") {
	DomOverDeg h(abs(rdz));
	add( h );
      } 
      else if( Heu == "domowldeg") {
	DomOverWLDeg h(abs(rdz));
	add( h );
      }
      else if( Heu == "domowdeg") {
	DomOverWDeg h(abs(rdz));
	add( h );
      }
      else if( Heu == "neighbor") {
	Neighbor h(abs(rdz));
	add( h );
      } 
      else if( Heu == "impact") {
	Impact h(abs(rdz));
	add( h );
      }
      else if( Heu == "impactodeg") {
	ImpactOverDeg h(abs(rdz));
	add( h );
      }
      else if( Heu == "impactowdeg") {
	ImpactOverWDeg h(abs(rdz));
	add( h );
      }
      else if( Heu == "impactowldeg") {
	ImpactOverWLDeg h(abs(rdz));
	add( h );
      }
      else if( Heu == "pfsp") {
	PFSP h(abs(rdz), val_ord);
	add( h );
      }
      else if( Heu == "osp") {
	OSP h(abs(rdz), val_ord);
	add( h );
      }
      else if( Heu == "osp-dw") {
	OSP h(abs(rdz), val_ord, OSP::DOMAIN_P_TWEIGHT);
	add( h );
      }
      else if( Heu == "osp-d") {
	OSP h(abs(rdz), val_ord, OSP::DOMAIN_O_NOT);
	add( h );
      }
      else if( Heu == "osp-b") {
	OSP h(abs(rdz), val_ord, OSP::DOM_O_BOOLWEIGHT);
	add( h );
      }
      else if( Heu == "osp-t") {
	OSP h(abs(rdz), val_ord, OSP::DOM_O_TASKWEIGHT);
	add( h );
      }
      else if( Heu == "osp-bt") {
	OSP h(abs(rdz), val_ord, OSP::DOM_O_BOOLTASKWEIGHT);
	add( h );
      }
//       else if( Heu == "osp-w") {
// 	OSP h(abs(rdz), val_ord, OSP_Type);
// 	add( h );
//       }
      else {
	NoOrder h;
	add( h );
      }
    }
    

    void dichotomic_search();
    //void branch_and_bound();

  };






}
