/** @file perturbations.h Documented includes for perturbation module */

#ifndef __PERTURBATIONS__
#define __PERTURBATIONS__

#include "thermodynamics.h"
#include "evolver_ndf15.h"
#include "evolver_rkck.h"

#define _scalars_ ((ppt->has_scalars == _TRUE_) && (index_md == ppt->index_md_scalars))
#define _vectors_ ((ppt->has_vectors == _TRUE_) && (index_md == ppt->index_md_vectors))
#define _tensors_ ((ppt->has_tensors == _TRUE_) && (index_md == ppt->index_md_tensors))

/**  
 * flags for various approximation schemes 
 * (tca = tight-coupling approximation, 
 *  rsa = radiation streaming approximation, 
 *  ufa = massless neutrinos / ultra-relativistic relics fluid approximation)
 *
 * CAUTION: must be listed below in chronological order, and cannot be
 * reversible. When integrating equations for a given mode, it is only
 * possible to switch from left to right in the lists below.
 */

//@{

enum tca_flags {tca_on, tca_off};
enum rsa_flags {rsa_off, rsa_on};
enum ufa_flags {ufa_off, ufa_on};
enum ncdmfa_flags {ncdmfa_off, ncdmfa_on};

//@}

/**
 * labels for the way in which each approximation scheme is implemented
 */

//@{

enum tca_method {first_order_MB,first_order_CAMB,first_order_CLASS,second_order_CRS,second_order_CLASS,compromise_CLASS};
enum rsa_method {rsa_null,rsa_MD,rsa_MD_with_reio,rsa_none};
enum ufa_method {ufa_mb,ufa_hu,ufa_CLASS,ufa_none};
enum ncdmfa_method {ncdmfa_mb,ncdmfa_hu,ncdmfa_CLASS,ncdmfa_none};

//@}

/** 
 * List of coded gauges. More gauges can in principle be defined. 
 */

//@{

enum possible_gauges {
  newtonian, /**< newtonian (or longitudinal) gauge */
  synchronous /**< synchronous gauge with \f$ \theta_{cdm} = 0 \f$ by convention */
};

//@}

//@{

/**
 * maximumu number and types of selection function (for bins of matter density or cosmic shear)
 */
#define _SELECTION_NUM_MAX_ 100
enum selection_type {gaussian,tophat,dirac};

//@}

/**
 * Structure containing everything about perturbations that other
 * modules need to know, in particular tabuled values of the source
 * functions \f$ S(k, \tau) \f$ for all requested modes
 * (scalar/vector/tensor), initial conditions, types (temperature,
 * E-polarization, B-polarisation, lensing potential, etc), multipole
 * l and wavenumber k.
 *
 */

struct perturbs
{
  /** @name - input parameters initialized by user in input module
   *  (all other quantitites are computed in this module, given these
   *  parameters and the content of the 'precision', 'background' and
   *  'thermodynamics' structures) */
  
  //@{

  short has_perturbations; /**< do we need to compute perturbations at all ? */

  short has_cls; /**< do we need any harmonic space spectrum C_l (and hence Bessel functions, transfer functions, ...)? */

  short has_scalars; /**< do we need scalars? */
  short has_vectors; /**< do we need vectors? */
  short has_tensors; /**< do we need tensors? */

  short has_ad;      /**< do we need adiabatic mode? */
  short has_bi;      /**< do we need isocurvature bi mode? */
  short has_cdi;     /**< do we need isocurvature cdi mode? */
  short has_nid;     /**< do we need isocurvature nid mode? */
  short has_niv;     /**< do we need isocurvature niv mode? */

  short has_cl_cmb_temperature;       /**< do we need Cl's for CMB temperature? */
  short has_cl_cmb_polarization;      /**< do we need Cl's for CMB polarization? */
  short has_cl_cmb_lensing_potential; /**< do we need Cl's for CMB lensing potential? */
  short has_cl_lensing_potential;     /**< do we need Cl's for galaxy lensing potential? */ 
  short has_cl_density;               /**< do we need Cl's for matter density? */
  short has_pk_matter;                /**< do we need matter Fourier spectrum? */
  short has_density_transfers;        /**< do we need to output individual matter density transfer functions? */
  short has_velocity_transfers;       /**< do we need to output individual matter velocity transfer functions? */

  int l_scalar_max; /**< maximum l value for CMB scalars C_ls */
  int l_tensor_max; /**< maximum l value for CMB tensors C_ls */
  int l_lss_max; /**< maximum l value for LSS C_ls (density and lensing potential in  bins) */
  double k_scalar_kmax_for_pk; /**< maximum value of k in 1/Mpc in P(k) (if scalar C_ls also requested, overseeded by value kmax inferred from l_scalar_max if it is bigger) */

  int selection_num;                            /**< number of selection functions 
						     (i.e. bins) for matter density Cls */
  enum selection_type selection;                /**< type of selection functions */
  double selection_mean[_SELECTION_NUM_MAX_]; /**< centers of selection functions */
  double selection_width[_SELECTION_NUM_MAX_];  /**< widths of selection functions */

  //@}

/** Here we introduce things used in LVDM model */


  double alpha; /**< lvdm parameter */
  double beta; 	/**< lvdm parameter */
  double lambda; /**< lvdm parameter */
  double Y_dm; /**< lvdm parameter */
  short has_lvdm;

  double G_cosmo_over_G_0;   
  double B;
  double H_alpha;
  double c_sq_chi;
  double k_y_0_sq;



  /** @name - useful flags infered from the ones above */

  //@{

  short has_cmb; /**< do we need CMB-related sources (temperature, polarization) ? */
  short has_lss; /**< do we need LSS-related sources (lensing potential, ...) ? */

  //@}

  /** @name - gauge in which to perform the calculation */

  //@{

  enum possible_gauges gauge; 

  //@}

  /** @name - indices running on modes (scalar, vector, tensor) */

  //@{

  int index_md_scalars; /**< index value for scalars */
  int index_md_tensors; /**< index value for tensors */
  int index_md_vectors; /**< index value for vectors */

  int md_size; /**< number of modes included in computation */

  //@}

  /** @name - indices running on initial conditions (for scalars: ad, cdi, nid, niv; for tensors: only one) */

  //@{

  int index_ic_ad; /**< index value for adiabatic */
  int index_ic_cdi; /**< index value for CDM isocurvature */
  int index_ic_bi; /**< index value for baryon isocurvature */
  int index_ic_nid; /**< index value for neutrino density isocurvature */
  int index_ic_niv; /**< index value for neutrino velocity isocurvature */
  int index_ic_ten; /**< index value for unique possibility for tensors */

  int * ic_size;       /**< for a given mode, ic_size[index_md] = number of initial conditions included in computation */

  //@}

  /** @name - flags and indices running on types (temperature, polarization, lensing, ...) */

  //@{

  short has_source_t;  /**< do we need source for CMB temperature? */
  short has_source_e;  /**< do we need source for CMB E-polarisation? */
  short has_source_b;  /**< do we need source for CMB B-polarisation? */
  short has_source_g;  /**< do we need source for gravitationnal potential? */
  short has_source_delta_pk; /**< do we need source for delta total? */ 
  short has_source_delta_g;   /**< do we need source for delta of gammas? */
  short has_source_delta_b;   /**< do we need source for delta of baryons? */
  short has_source_delta_cdm; /**< do we need source for delta of cold dark matter? */
  short has_source_delta_fld;  /**< do we need source for delta of dark energy? */
  short has_source_delta_ur; /**< do we need source for delta of ultra-relativistic neutrinos/relics? */
  short has_source_delta_ncdm; /**< do we need source for delta of all non-cold dark matter species (e.g. massive neutrinos)? */
  short has_source_theta_g;   /**< do we need source for theta of gammas? */
  short has_source_theta_b;   /**< do we need source for theta of baryons? */
  short has_source_theta_cdm; /**< do we need source for theta of cold dark matter? */
  short has_source_theta_fld;  /**< do we need source for theta of dark energy? */
  short has_source_theta_ur; /**< do we need source for theta of ultra-relativistic neutrinos/relics? */
  short has_source_theta_ncdm; /**< do we need source for theta of all non-cold dark matter species (e.g. massive neutrinos)? */

  int index_tp_t; /**< index value for temperature */
  int index_tp_e; /**< index value for E-polarization */
  int index_tp_b; /**< index value for B-polarization */
  int index_tp_g; /**< index value for gravitationnal potential */
  int index_tp_delta_pk; /**< index value for delta tot */
  int index_tp_delta_g;   /**< index value for delta of gammas */
  int index_tp_delta_b;   /**< index value for delta of baryons */
  int index_tp_delta_cdm; /**< index value for delta of cold dark matter */
  int index_tp_delta_fld;  /**< index value for delta of dark energy */
  int index_tp_delta_ur; /**< index value for delta of ultra-relativistic neutrinos/relics */
  int index_tp_delta_ncdm1; /**< index value for delta of first non-cold dark matter species (e.g. massive neutrinos) */
  int index_tp_theta_g;   /**< index value for theta of gammas */
  int index_tp_theta_b;   /**< index value for theta of baryons */
  int index_tp_theta_cdm; /**< index value for theta of cold dark matter */
  int index_tp_theta_fld;  /**< index value for theta of dark energy */
  int index_tp_theta_ur; /**< index value for theta of ultra-relativistic neutrinos/relics */
  int index_tp_theta_ncdm1; /**< index value for theta of first non-cold dark matter species (e.g. massive neutrinos) */
  int * tp_size; /**< number of types tp_size[index_md] included in computation for each mode */

  //@}

  /** @name - list of k values for each mode */
  
  //@{

  int * k_size_cmb;  /**< k_size_cmb[index_md] number of k values used
			for CMB calculations, requiring a fine
			sampling in k-space */

  int * k_size_cl;  /**< k_size_cl[index_md] number of k values used
		       for non-CMB Cl calculations, requering a coarse
		       sampling in k-space. */

  int * k_size;     /**< k_size[index_md] = total number of k
		       values, including those needed for P(k) but not
		       for Cl's */

  double ** k;      /**< k[index_md][index_k] = list of values */

  //@}

  /** @name - list of conformal time values in the source table
      (common to all modes and types) */

  //@{

  int tau_size;          /**< tau_size = number of values */

  double * tau_sampling; /**< tau_sampling[index_tau] = list of tau values */

  double selection_min_of_tau_min; /**< used in presence of selection functions (for matter density, cosmic shear...) */
  double selection_max_of_tau_max; /**< used in presence of selection functions (for matter density, cosmic shear...) */

  double selection_delta_tau; /**< used in presence of selection functions (for matter density, cosmic shear...) */

  double * selection_tau_min; /**< value of conformal time below which W(tau) is considered to vanish for each bin */
  double * selection_tau_max; /**< value of conformal time above which W(tau) is considered to vanish for each bin */
  double * selection_tau; /**< value of conformal time at the center of each bin */
  double * selection_function; /** selection function W(tau), normalized to \int W(tau) dtau=1, stored in selection_function[bin*ppt->tau_size+index_tau] */ 

  //@}

  /** @name - source functions interpolation table */

  //@{

  double *** sources; /**< Pointer towards the source interpolation table
			 sources[index_md]
			 [index_ic * ppt->tp_size[index_md] + index_type]
			 [index_tau * ppt->k_size[index_md] + index_k] */


  //@}

  /** @name - technical parameters */

  //@{

  short perturbations_verbose; /**< flag regulating the amount of information sent to standard output (none if set to zero) */

  ErrorMsg error_message; /**< zone for writing error messages */

  //@}

};

/**
 * Structure containing the indices and the values of the perturbation
 * variables which are integrated over time (as well as their
 * time-derivatives). For a given wavenumber, the size of these
 * vectors changes when the approximation scheme changes.
 */

struct perturb_vector
{
  int index_pt_delta_g;   /**< photon density */
  int index_pt_theta_g;   /**< photon velocity */
  int index_pt_shear_g;   /**< photon shear */
  int index_pt_l3_g;      /**< photon l=3 */
  int l_max_g;            /**< max momentum in Boltzmann hierarchy (at least 3) */
  int index_pt_pol0_g;    /**< photon polarization, l=0 */
  int index_pt_pol1_g;    /**< photon polarization, l=1 */
  int index_pt_pol2_g;    /**< photon polarization, l=2 */
  int index_pt_pol3_g;    /**< photon polarization, l=3 */
  int l_max_pol_g;        /**< max momentum in Boltzmann hierarchy (at least 3) */
  int index_pt_delta_b;   /**< baryon density */
  int index_pt_theta_b;   /**< baryon velocity */
  int index_pt_delta_cdm; /**< cdm density */
  int index_pt_theta_cdm; /**< cdm velocity */
  int index_pt_delta_fld;  /**< dark energy density */
  int index_pt_theta_fld;  /**< dark energy velocity */
  int index_pt_delta_ur; /**< density of ultra-relativistic neutrinos/relics */
  int index_pt_theta_ur; /**< velocity of ultra-relativistic neutrinos/relics */
  int index_pt_shear_ur; /**< shear of ultra-relativistic neutrinos/relics */
  int index_pt_l3_ur;    /**< l=3 of ultra-relativistic neutrinos/relics */
  int l_max_ur;          /**< max momentum in Boltzmann hierarchy (at least 3) */
  int index_pt_chi;       /**< khronon */
  int index_pt_chidot;    /**< khronon variation */
  int index_pt_psi0_ncdm1;
  int N_ncdm;
  int* l_max_ncdm;
  int* q_size_ncdm;

  int index_pt_eta;       /**< synchronous gauge metric perturbation eta*/
  int index_pt_phi;
  int index_pt_gw;        /**< tensor metric perturbation h (gravitational waves) */
  int index_pt_gwdot;     /**< its time-derivative */
  int pt_size;            /**< size of perturbation vector */

  double * y;             /**< vector of perturbations to be integrated */
  double * dy;            /**< time-derivative of the same vector */

  int * used_in_sources; /**< boolean array specifying which
			      perturbations enter in the calculation of
			      source functions */
 
};


/**
 * Workspace containing, among other things, the value at a given time
 * of all background/perturbed quantitites, as well as their indices.
 *
 * There will be one such structure created for each mode
 * (scalar/.../tensor) and each thread (in case of parallel computing)
 */

struct perturb_workspace 
{

  /** @name - all possible useful indices for those metric
      perturbations which are not integrated over time, but just
      inferred from Einstein equations. "_mt_" stands for "metric".*/

  //@{

  int index_mt_phi;         /**< phi in longitudinal gauge */
  int index_mt_psi;         /**< psi in longitudinal gauge */
  int index_mt_phi_prime;   /**< (d phi/d conf.time) in longitudinal gauge */
  int index_mt_h_prime;     /**< h' (wrt conf. time) in synchronous gauge */
  int index_mt_h_prime_prime; /**< h'' (wrt conf. time) in synchronous gauge */
  int index_mt_eta_prime;   /**< eta' (wrt conf. time) in synchronous gauge */
  int index_mt_alpha_prime; /**< (d \f$ \alpha \f$/d conf.time) in synchronous gauge, where \f$ \alpha = (h' + 6 \eta') / (2 k^2) \f$ */
  int mt_size;              /**< size of metric perturbation vector */

  //@}

  /** @name - all possible useful indices for terms contributing to
      the source function S=S0+S1'+S2''. These quantitites are not
      integrated over time, but to inferred from the "_pt_" and "_mt_"
      vectors, and their time derivatives. "_st_" stands for "source
      term".
   */

  //@{

  int index_st_tau;    /**< conformal time */
  int index_st_S0;     /**< first piece S0 */
  int index_st_S1;     /**< second piece S1 */
  int index_st_S2;     /**< third piece S2 */
  int index_st_dS1;    /**< derivative S1' */
  int index_st_dS2;    /**< derivative S2' */
  int index_st_ddS2;   /**< derivative S2'' */
  int st_size;         /**< size of this vector */ 
  
  //@}

  /** @name - value at a given time of all background/perturbed
      quantitites
   */

  //@{

  double * pvecback;          /**< background quantitites */
  double * pvecthermo;        /**< thermodynamics quantitites */
  double * pvecmetric;        /**< metric quantitites */
  struct perturb_vector * pv; /**< pointer to vector of integrated
				 perturbations and their
				 time-derivatives */

  double tca_shear_g; /**< photon shear in tight-coupling approximation */
  double tca_shear_g_prime; /**< photon shear derivative in tight-coupling approximation */
  double rsa_delta_g; /**< photon density in radiation streaming approximation */
  double rsa_theta_g; /**< photon velocity in radiation streaming approximation */
  double rsa_delta_ur; /**< photon density in radiation streaming approximation */
  double rsa_theta_ur; /**< photon velocity in radiation streaming approximation */

  double * delta_ncdm;
  double * theta_ncdm;
  double * shear_ncdm;

  double delta_pk;

  //@}

  /** @name - table of source terms for each mode, initial condition
      and wavenumber: source_term_table[index_type][index_tau*ppw->st_size+index_st] */

  //@{

  double ** source_term_table;

  //@}

  /** @name - indices useful for searching background/termo quantitites in tables */

  //@{

  short inter_mode;
 
  int last_index_back;   /**< the background interpolation function background_at_tau() keeps memory of the last point called through this index */
  int last_index_thermo; /**< the thermodynamics interpolation function thermodynamics_at_z() keeps memory of the last point called through this index */

  //@}

  /** @name - approximations used at a given time */

  //@{

  int index_ap_tca; /**< index for tight-coupling approximation */
  int index_ap_rsa; /**< index for radiation streaming approximation */
  int index_ap_ufa; /**< index for ur fluid approximation */  
  int index_ap_ncdmfa; /**< index for ncdm fluid approximation */
  int ap_size;      /**< number of relevant approximations for a given mode */

  int * approx;     /**< array of approximation flags holding at a given time: approx[index_ap] */

  //@}

};

/**
 * Structure pointing towards all what the function that perturb_derivs
 * needs to know: fixed input parameters and indices contained in the
 * various structures, workspace, etc.
*/ 

struct perturb_parameters_and_workspace {

  struct precision * ppr;         /**< pointer to the precision structure */
  struct background * pba;        /**< pointer to the background structure */
  struct thermo * pth;            /**< pointer to the thermodynamics structure */
  struct perturbs * ppt;          /**< pointer to the precision structure */
  int index_md;                 /**< index of mode (scalar/.../vector/tensor) */
  double k;
  struct perturb_workspace * ppw; /**< worspace defined above */
  
};

/*************************************************************************************************************/

/*
 * Boilerplate for C++ 
 */
#ifdef __cplusplus
  extern "C" {
#endif

    int perturb_sources_at_tau(
			       struct perturbs * ppt,
			       int index_md,
			       int index_ic,
			       int index_type,
			       double tau,
			       double * pvecsources
			       );

    int perturb_init(
		     struct precision * ppr,
		     struct background * pba,
		     struct thermo * pth,
		     struct perturbs * ppt
		     );

    int perturb_free(
		     struct perturbs * ppt
		     );

    int perturb_indices_of_perturbs(
				    struct precision * ppr,
				    struct background * pba,
				    struct thermo * pth,
				    struct perturbs * ppt
				    );

    int perturb_timesampling_for_sources(
					 struct precision * ppr,
					 struct background * pba,
					 struct thermo * pth,
					 struct perturbs * ppt
					 );
    int perturb_get_k_list(
			   struct precision * ppr,
			   struct background * pba,
			   struct thermo * pth,
			   struct perturbs * ppt,
			   int index_md);

    int perturb_workspace_init(
			       struct precision * ppr,
			       struct background * pba,
			       struct thermo * pth,
			       struct perturbs * ppt,
			       int index_md,
			       struct perturb_workspace * ppw
			       );

    int perturb_workspace_free(
			       struct perturbs * ppt,
			       int index_md,
			       struct perturb_workspace * ppw
			       );

    int perturb_solve(
		      struct precision * ppr,
		      struct background * pba,
		      struct thermo * pth,
		      struct perturbs * ppt,
		      int index_md,
		      int index_ic,
		      int index_k,
		      struct perturb_workspace * ppw
		      );

    int perturb_find_approximation_number(
					  struct precision * ppr,
					  struct background * pba,
					  struct thermo * pth,
					  struct perturbs * ppt,
					  int index_md,
					  double k,
					  struct perturb_workspace * ppw,
					  double tau_ini,
					  double tau_end,
					  int * interval_number,
					  int * interval_number_of
					  );

    int perturb_find_approximation_switches(
					    struct precision * ppr,
					    struct background * pba,
					    struct thermo * pth,
					    struct perturbs * ppt,
					    int index_md,
					    double k,
					    struct perturb_workspace * ppw,
					    double tau_ini,
					    double tau_end,
					    double precision,
					    int interval_number,
					    int * interval_number_of,
					    double * interval_limit,
					    int ** interval_approx
					    );

    int perturb_vector_init(
			    struct precision * ppr,
			    struct background * pba,
			    struct thermo * pth,
			    struct perturbs * ppt,
			    int index_md,
			    int index_ic,
			    double k,
			    double tau,
			    struct perturb_workspace * ppw,
			    int * pa_old
			    );

    int perturb_vector_free(
			    struct perturb_vector * pv
			    );

    int perturb_initial_conditions(
				   struct precision * ppr,
				   struct background * pba,
				   struct perturbs * ppt,
				   int index_md,
				   int index_ic,
				   double k,
				   double tau,
				   struct perturb_workspace * ppw
				   );

    int perturb_approximations(
			       struct precision * ppr,
			       struct background * pba,
			       struct thermo * pth,
			       struct perturbs * ppt,
			       int index_md,
			       double k,
			       double tau,
			       struct perturb_workspace * ppw
			       );

    int perturb_timescale(
			  double tau,
			  void * parameters_and_workspace,
			  double * timescale,
			  ErrorMsg error_message
			  );

    int perturb_einstein(
			 struct precision * ppr,
			 struct background * pba,
			 struct thermo * pth,
			 struct perturbs * ppt,
			 int index_md,
			 double k,
			 double tau,
			 double * y,
			 struct perturb_workspace * ppw
			 );

    int perturb_source_terms(
			     double tau,
			     double * pvecperturbations,
			     double * pvecderivs,
			     int index_tau,
			     void * parameters_and_workspace,
			     ErrorMsg error_message
			     );

    int perturb_sources(
			struct precision * ppr,
			struct perturbs * ppt,
			int index_md,
			int index_ic,
			int index_k,
			struct perturb_workspace * ppw
			);

    int perturb_print_variables(double tau,
				double * y,
				double * dy,
				void * parameters_and_workspace,
				ErrorMsg error_message
				);

    int perturb_derivs(
		       double tau,
		       double * y,
		       double * dy,
		       void * parameters_and_workspace,
		       ErrorMsg error_message
		       );

    
#ifdef __cplusplus
  }
#endif

/**************************************************************/

#endif
