/** @file input.c Documented input module.
 *
 * Julien Lesgourgues, 27.08.2010    
 */

#include "input.h" 

/**
 * Use this routine to extract initial parameters from files 'xxx.ini'
 * and/or 'xxx.pre'. They can be the arguments of the main() routine.
 *
 * If class is embedded into another code, you will probably prefer to
 * call directly input_init() in order to pass input parameters
 * through a 'file_content' structure.
 */

int input_init_from_arguments(
                              int argc, 
                              char **argv,
                              struct precision * ppr,
                              struct background *pba,
                              struct thermo *pth,
                              struct perturbs *ppt,
                              struct bessels * pbs,
                              struct transfers *ptr,
                              struct primordial *ppm,
                              struct spectra *psp,
                              struct nonlinear * pnl,
                              struct lensing *ple,
                              struct output *pop,
                              ErrorMsg errmsg
                              ) {

  /** Summary: */

  /** - define local variables */

  struct file_content fc;
  struct file_content fc_input;
  struct file_content fc_precision;

  char input_file[_ARGUMENT_LENGTH_MAX_];
  char precision_file[_ARGUMENT_LENGTH_MAX_];

  int i;
  char extension[5];

  /** - Initialize the two file_content structures (for input
      parameters and precision parameters) to some null content. If no
      arguments are passed, they will remain null and inform
      init_params() that all parameters take default values. */

  fc.size = 0;
  fc_input.size = 0;
  fc_precision.size = 0;
  input_file[0]='\0';
  precision_file[0]='\0';

  /** If some arguments are passed, identify eventually some 'xxx.ini'
      and 'xxx.pre' files, and store their name. */

  if (argc > 1) {
    for (i=1; i<argc; i++) {
      strncpy(extension,(argv[i]+strlen(argv[i])-4),4);
      extension[4]='\0';
      if (strcmp(extension,".ini") == 0) {
        class_test(input_file[0] != '\0',
                   errmsg,
                   "You have passed more than one input file with extension '.ini', choose one.");
        strcpy(input_file,argv[i]);
      }
      if (strcmp(extension,".pre") == 0) {
        class_test(precision_file[0] != '\0',
                   errmsg,
                   "You have passed more than one precision with extension '.pre', choose one.");
        strcpy(precision_file,argv[i]);
      }
    }
  }
  
  /** - if there is an 'xxx.ini' file, read it and store its content. */

  if (input_file[0] != '\0')
    
    class_call(parser_read_file(input_file,&fc_input,errmsg),
               errmsg,
               errmsg);

  /** - if there is an 'xxx.pre' file, read it and store its content. */

  if (precision_file[0] != '\0')
    
    class_call(parser_read_file(precision_file,&fc_precision,errmsg),
               errmsg,
               errmsg);

  /** - if one or two files were read, merge their contents in a
      single 'file_content' structure. */

  if ((input_file[0]!='\0') || (precision_file[0]!='\0'))

    class_call(parser_cat(&fc_input,&fc_precision,&fc,errmsg),
               errmsg,
               errmsg);

  class_call(parser_free(&fc_input),errmsg,errmsg);
  class_call(parser_free(&fc_precision),errmsg,errmsg);
  
  /** - now, initialize all parameters given the input 'file_content'
      structure.  If its size is null, all parameters take their
      default values. */

  class_call(input_init(&fc,
                        ppr,
                        pba,
                        pth,
                        ppt,
                        pbs,
                        ptr,
                        ppm,
                        psp,
                        pnl,
                        ple,
                        pop,
                        errmsg),
             errmsg,
             errmsg);
  
  class_call(parser_free(&fc),errmsg,errmsg);

  return _SUCCESS_;
}

/**
 * Initialize each parameters, first to its default values, and then
 * from what can be interpreted from the values passed in the input
 * 'file_content' structure. If its size is null, all parameters keep
 * their default values.
 */

int input_init(
               struct file_content * pfc,
               struct precision * ppr,
               struct background *pba,
               struct thermo *pth,
               struct perturbs *ppt,
               struct bessels * pbs,
               struct transfers *ptr,
               struct primordial *ppm,
               struct spectra *psp,
               struct nonlinear * pnl,
               struct lensing *ple,
               struct output *pop,
               ErrorMsg errmsg
               ) {

  /** Summary: */

  /** - define local variables */

  int flag1,flag2,flag3;
  double param1,param2,param3;
  int N_ncdm=0,n,entries_read;
  int int1,fileentries;
  double fnu_factor;
  double * pointer1;
  char string1[_ARGUMENT_LENGTH_MAX_];
  double k1=0.;
  double k2=0.;
  double prr1=0.;
  double prr2=0.;
  double pii1=0.;
  double pii2=0.;
  double pri1=0.;
  double pri2=0.;
  double n_iso=0.;
  double f_iso=0.;
  double n_cor=0.;
  double c_cor=0.;

  double Omega_tot;

  int i;

  FILE * param_output;
  FILE * param_unused;
  char param_output_name[_LINE_LENGTH_MAX_];
  char param_unused_name[_LINE_LENGTH_MAX_];

  double sigma_B; /**< Stefan-Boltzmann constant in W/m^2/K^4 = Kg/K^4/s^3 */

  double rho_ncdm;
  double R0,R1,R2,R3,R4;

  sigma_B = 2. * pow(_PI_,5) * pow(_k_B_,4) / 15. / pow(_h_P_,3) / pow(_c_,2);

  /** - set all parameters (input and precision) to default values */

  class_call(input_default_params(pba,
                                  pth,
                                  ppt,
                                  pbs,
                                  ptr,
                                  ppm,
                                  psp,
                                  pnl,
                                  ple,
                                  pop),
             errmsg,
             errmsg);

  class_call(input_default_precision(ppr),
             errmsg,
             errmsg);

  /** - if entries passed in file_content structure, carefully read
      and interpret each of them, and tune accordingly the relevant
      input parameters */

  /** (a) background parameters */

  /* h (dimensionless) and [H0/c] in Mpc^{-1} = h / 2999.7 */
  class_call(parser_read_double(pfc,"H0",&param1,&flag1,errmsg),
             errmsg,
             errmsg);
  class_call(parser_read_double(pfc,"h",&param2,&flag2,errmsg),
             errmsg,
             errmsg);
  class_test((flag1 == _TRUE_) && (flag2 == _TRUE_),
             errmsg,
             "In input file, you cannot enter both h and H0, choose one");
  if (flag1 == _TRUE_) {
    pba->H0 = param1 * 1.e3 / _c_;
    pba->h = param1 / 100.;
  }
  if (flag2 == _TRUE_) {
    pba->H0 = param2 *  1.e5 / _c_;
    pba->h = param2;
  }

  /* Omega_0_g (photons) and T_cmb */
  class_call(parser_read_double(pfc,"T_cmb",&param1,&flag1,errmsg),
             errmsg,
             errmsg);
  class_call(parser_read_double(pfc,"Omega_g",&param2,&flag2,errmsg),
             errmsg,
             errmsg);
  class_call(parser_read_double(pfc,"omega_g",&param3,&flag3,errmsg),
             errmsg,
             errmsg);
  class_test(class_at_least_two_of_three(flag1,flag2,flag3),
             errmsg,
             "In input file, you can only enter one of T_cmb, Omega_g or omega_g, choose one");

  if (class_none_of_three(flag1,flag2,flag3)) {
    pba->Omega0_g = (4.*sigma_B/_c_*pow(pba->T_cmb,4.)) / (3.*_c_*_c_*1.e10*pba->h*pba->h/_Mpc_over_m_/_Mpc_over_m_/8./_PI_/_G_);
  }
  else {

    if (flag1 == _TRUE_) {
      /* Omega0_g = rho_g / rho_c0, each of them expressed in Kg/m/s^2 */
      /* rho_g = (4 sigma_B / c) T^4 */
      /* rho_c0 = 3 c^2 H0^2 / (8 pi G) */ 
      pba->Omega0_g = (4.*sigma_B/_c_*pow(param1,4.)) / (3.*_c_*_c_*1.e10*pba->h*pba->h/_Mpc_over_m_/_Mpc_over_m_/8./_PI_/_G_);
      pba->T_cmb=param1;
    }

    if (flag2 == _TRUE_) {
      pba->Omega0_g = param2;
      pba->T_cmb=pow(pba->Omega0_g * (3.*_c_*_c_*1.e10*pba->h*pba->h/_Mpc_over_m_/_Mpc_over_m_/8./_PI_/_G_) / (4.*sigma_B/_c_),0.25);
    }

    if (flag3 == _TRUE_) {
      pba->Omega0_g = param3/pba->h/pba->h;
      pba->T_cmb = pow(pba->Omega0_g * (3.*_c_*_c_*1.e10*pba->h*pba->h/_Mpc_over_m_/_Mpc_over_m_/8./_PI_/_G_) / (4.*sigma_B/_c_),0.25);
    }
  }

  Omega_tot = pba->Omega0_g;

  /* Omega_0_b (baryons) */
  class_call(parser_read_double(pfc,"Omega_b",&param1,&flag1,errmsg),
             errmsg,
             errmsg);
  class_call(parser_read_double(pfc,"omega_b",&param2,&flag2,errmsg),
             errmsg,
             errmsg);
  class_test(((flag1 == _TRUE_) && (flag2 == _TRUE_)),
             errmsg,
             "In input file, you can only enter one of Omega_b or omega_b, choose one");
  if (flag1 == _TRUE_)
    pba->Omega0_b = param1;
  if (flag2 == _TRUE_)
    pba->Omega0_b = param2/pba->h/pba->h;

  Omega_tot += pba->Omega0_b;

  /* Omega_0_ur (ultra-relativistic species / massless neutrino) */
  class_call(parser_read_double(pfc,"N_eff",&param1,&flag1,errmsg),
             errmsg,
             errmsg);
  class_call(parser_read_double(pfc,"Omega_ur",&param2,&flag2,errmsg),
             errmsg,
             errmsg);
  class_call(parser_read_double(pfc,"omega_ur",&param3,&flag3,errmsg),
             errmsg,
             errmsg);
  class_test(class_at_least_two_of_three(flag1,flag2,flag3),
             errmsg,
             "In input file, you can only enter one of N_eff, Omega_ur or omega_ur, choose one");

  if (class_none_of_three(flag1,flag2,flag3)) {
    pba->Omega0_ur = 3.04*7./8.*pow(4./11.,4./3.)*pba->Omega0_g;
  }
  else {

    if (flag1 == _TRUE_) {
      pba->Omega0_ur = param1*7./8.*pow(4./11.,4./3.)*pba->Omega0_g;
    }
    if (flag2 == _TRUE_) {
      pba->Omega0_ur = param2;
    }
    if (flag3 == _TRUE_) {
      pba->Omega0_ur = param3/pba->h/pba->h;
    }
  }

  Omega_tot += pba->Omega0_ur;

  /* Omega_0_cdm (CDM) */
  class_call(parser_read_double(pfc,"Omega_cdm",&param1,&flag1,errmsg),
             errmsg,
             errmsg);
  class_call(parser_read_double(pfc,"omega_cdm",&param2,&flag2,errmsg),
             errmsg,
             errmsg);
  class_test(((flag1 == _TRUE_) && (flag2 == _TRUE_)),
             errmsg,
             "In input file, you can only enter one of Omega_cdm or omega_cdm, choose one");
  if (flag1 == _TRUE_)
    pba->Omega0_cdm = param1;
  if (flag2 == _TRUE_)
    pba->Omega0_cdm = param2/pba->h/pba->h;

  Omega_tot += pba->Omega0_cdm;

  /* non-cold relics (ncdm) */
  class_read_int("N_ncdm",N_ncdm);
  if ((flag1 == _TRUE_) && (N_ncdm > 0)){	
    pba->N_ncdm = N_ncdm;
    /* Precision parameters for ncdm has to be read now since they are used here:*/
    class_read_double("tol_M_ncdm",ppr->tol_M_ncdm);
    class_read_double("tol_ncdm",ppr->tol_ncdm);
    class_read_double("tol_ncdm_bg",ppr->tol_ncdm_bg);
	
    /* Read temperatures: */
    class_read_list_of_doubles_or_default("T_ncdm",pba->T_ncdm,pba->T_ncdm_default,N_ncdm);

    /* Read chemical potentials: */
    class_read_list_of_doubles_or_default("ksi_ncdm",pba->ksi_ncdm,pba->ksi_ncdm_default,N_ncdm);

    /* Read degeneracy of each ncdm species: */
    class_read_list_of_doubles_or_default("deg_ncdm",pba->deg_ncdm,pba->deg_ncdm_default,N_ncdm);

    /* Read mass of each ncdm species: */
    class_read_list_of_doubles_or_default("m_ncdm",pba->m_ncdm_in_eV,0.0,N_ncdm);

    /* Read Omega of each ncdm species: */
    class_read_list_of_doubles_or_default("Omega_ncdm",pba->Omega0_ncdm,0.0,N_ncdm);

    /* Read omega of each ncdm species: (Use pba->M_ncdm temporarily)*/
    class_read_list_of_doubles_or_default("omega_ncdm",pba->M_ncdm,0.0,N_ncdm);

    /* Check for duplicate Omega/omega entries, missing mass definition and 
       update pba->Omega0_ncdm:*/
    for(n=0; n<N_ncdm; n++){
      /* pba->M_ncdm holds value of omega */
      if (pba->M_ncdm[n]!=0.0){
        class_test(pba->Omega0_ncdm[n]!=0,errmsg,
                   "Nonzero values for both Omega and omega for ncdm species %d are specified!",n);
        pba->Omega0_ncdm[n] = pba->M_ncdm[n]/pba->h/pba->h;
      }
      if ((pba->Omega0_ncdm[n]==0.0) && (pba->m_ncdm_in_eV[n]==0.0)) {
        /* this is the right place for passing the default value of
           the mass (all parameters must have a default value; most of
           them are defined in input_default_params{}, but the ncdm mass
           is a bit special and there is no better place for setting its
           default value). We put an aribitrary value m << 10^-3 eV,
           i.e. the ultra-relativistic limit.*/
        pba->m_ncdm_in_eV[n]=1.e-5;
      }
    }

    /* Check if filenames for interpolation tables are given: */
    class_read_list_of_integers_or_default("use_ncdm_psd_files",pba->got_files,_FALSE_,N_ncdm);
	
    if (flag1==_TRUE_){
      for(n=0,fileentries=0; n<N_ncdm; n++){
        if (pba->got_files[n] == _TRUE_) fileentries++;
      }

      if (fileentries > 0) {

        /* Okay, read filenames.. */
        class_call(parser_read_list_of_strings(pfc,"ncdm_psd_filenames",
                                               &entries_read,&(pba->ncdm_psd_files),&flag2,errmsg),
                   errmsg,
                   errmsg);
        class_test(flag2 == _FALSE_,errmsg, 
                   "Input use_ncdm_files is found, but no filenames found!");
        class_test(entries_read != fileentries,errmsg,
                   "Numer of filenames found, %d, does not match number of _TRUE_ values in use_ncdm_files, %d",
                   entries_read,fileentries);
      }
    }
    /* Read (optional) p.s.d.-parameters:*/
    parser_read_list_of_doubles(pfc,
                                "ncdm_psd_parameters",
                                &entries_read,
                                &(pba->ncdm_psd_parameters),
                                &flag2,
                                errmsg);

    class_call(background_ncdm_init(ppr,pba),
               pba->error_message,
               errmsg);
	
    /* We must calculate M from omega or vice versa if one of them is missing.
       If both are present, we must update the degeneracy parameter to
       reflect the implicit normalisation of the distribution function.*/
    for (n=0; n < N_ncdm; n++){
      if (pba->m_ncdm_in_eV[n] != 0.0){
        /* Case of only mass or mass and Omega/omega: */
        pba->M_ncdm[n] = pba->m_ncdm_in_eV[n]/_k_B_*_eV_/pba->T_ncdm[n]/pba->T_cmb;
        class_call(background_ncdm_momenta(pba->q_ncdm_bg[n],
                                           pba->w_ncdm_bg[n],
                                           pba->q_size_ncdm_bg[n],
                                           pba->M_ncdm[n],
                                           pba->factor_ncdm[n],
                                           0.,
                                           NULL,
                                           &rho_ncdm,
                                           NULL,
                                           NULL,
                                           NULL), 
                   pba->error_message,
                   errmsg);
        if (pba->Omega0_ncdm[n] == 0.0){
          pba->Omega0_ncdm[n] = rho_ncdm/pba->H0/pba->H0;
        }
        else{
          fnu_factor = (pba->H0*pba->H0*pba->Omega0_ncdm[n]/rho_ncdm);
          pba->factor_ncdm[n] *= fnu_factor;
          /* dlnf0dlnq is already computed, but it is 
             independent of any normalisation of f0.
             We don't need the factor anymore, but we
             store it nevertheless:*/
          pba->deg_ncdm[n] *=fnu_factor;
        }
      }
      else{
        /* Case of only Omega/omega: */
        class_call(background_ncdm_M_from_Omega(ppr,pba,n),
                   pba->error_message,
                   errmsg);
        //printf("M_ncdm:%g\n",pba->M_ncdm[n]);
        pba->m_ncdm_in_eV[n] = _k_B_/_eV_*pba->T_ncdm[n]*pba->M_ncdm[n]*pba->T_cmb;
      }
      pba->Omega0_ncdm_tot += pba->Omega0_ncdm[n];
      //printf("Adding %g to total Omega..\n",pba->Omega0_ncdm[n]);
    }			
  }
  Omega_tot += pba->Omega0_ncdm_tot;

  /* Omega_0_k (curvature) */
  class_read_double("Omega_k",pba->Omega0_k);

  /* Omega_0_lambda (cosmological constant), Omega0_fld (dark energy fluid) */
  class_call(parser_read_double(pfc,"Omega_Lambda",&param1,&flag1,errmsg),
             errmsg,
             errmsg);
  class_call(parser_read_double(pfc,"Omega_fld",&param2,&flag2,errmsg),
             errmsg,
             errmsg);
  class_test((flag1 == _TRUE_) && (flag2 == _TRUE_),
             errmsg,
             "In input file, you can enter only two out of Omega_Lambda, Omega_de, Omega_k, the third one is inferred");

  if ((flag1 == _FALSE_) && (flag2 == _FALSE_)) {	
    pba->Omega0_lambda = 1.+pba->Omega0_k-pba->Omega0_g-pba->Omega0_ur-pba->Omega0_b-pba->Omega0_cdm-pba->Omega0_ncdm_tot;
  }
  else {
    if (flag1 == _TRUE_) {
      pba->Omega0_lambda= param1;
      pba->Omega0_fld = 1. + pba->Omega0_k - param1 - Omega_tot;
    }
    if (flag2 == _TRUE_) {
      pba->Omega0_lambda= 1. + pba->Omega0_k - param2 - Omega_tot;
      pba->Omega0_fld = param2;
    }
  }

  if (pba->Omega0_fld != 0.) {
    class_read_double("w0_fld",pba->w0_fld);
    class_read_double("wa_fld",pba->wa_fld);
    class_read_double("cs2_fld",pba->cs2_fld);

    class_test(pba->w0_fld<=-1.,
               errmsg,
               "Your choice w_fld=%g is not valid, it will lead to instabilities or division by zero\n",
               pba->w0_fld);
	       
    class_test(pba->w0_fld+pba->w0_fld>=1./3.,
               errmsg,
               "Your choice for w0_fld+wa_fld=%g is suspicious, ther would not be radiation domination at early times\n",
               pba->w0_fld+pba->wa_fld);
	       

  }

  /* scale factor today (arbitrary) */
  class_read_double("a_today",pba->a_today);

  /** (b) assign values to thermodynamics cosmological parameters */

  /* primordial helium fraction */
  class_call(parser_read_string(pfc,"YHe",&string1,&flag1,errmsg),
             errmsg,
             errmsg);

  if (flag1 == _TRUE_) {
    
    if ((strstr(string1,"BBN") != NULL) || (strstr(string1,"bbn") != NULL)) {
      pth->YHe = _BBN_;  
    }
    else {
      class_read_double("YHe",pth->YHe);
    }
    
  }

  /* recombination parameters */
  class_call(parser_read_string(pfc,"recombination",&string1,&flag1,errmsg),
             errmsg,
             errmsg);

  if (flag1 == _TRUE_) {

    if ((strstr(string1,"HYREC") != NULL) || (strstr(string1,"hyrec") != NULL) || (strstr(string1,"HyRec") != NULL)) {
      pth->recombination = hyrec;  
    }

  }

  /* reionization parametrization */
  class_call(parser_read_string(pfc,"reio_parametrization",&string1,&flag1,errmsg),
             errmsg,
             errmsg);

  if (flag1 == _TRUE_) {
    flag2=_FALSE_;
    if (strcmp(string1,"reio_none") == 0) {
      pth->reio_parametrization=reio_none;
      flag2=_TRUE_;
    }
    if (strcmp(string1,"reio_camb") == 0) {
      pth->reio_parametrization=reio_camb;
      flag2=_TRUE_;
    }
    if (strcmp(string1,"reio_bins_tanh") == 0) {
      pth->reio_parametrization=reio_bins_tanh;
      flag2=_TRUE_;
    }
    if (strcmp(string1,"reio_half_tanh") == 0) {
      pth->reio_parametrization=reio_half_tanh;
      flag2=_TRUE_;
    }

    class_test(flag2==_FALSE_,
               errmsg,
               "could not identify reionization_parametrization value, check that it is one of 'reio_none', 'reio_camb', 'reio_bins_tanh', ...");
  }

  /* reionization parameters if reio_parametrization=reio_camb */
  if ((pth->reio_parametrization == reio_camb) || (pth->reio_parametrization == reio_half_tanh)){
    class_call(parser_read_double(pfc,"z_reio",&param1,&flag1,errmsg),
               errmsg,
               errmsg);
    class_call(parser_read_double(pfc,"tau_reio",&param2,&flag2,errmsg),
               errmsg,
               errmsg);
    class_test(((flag1 == _TRUE_) && (flag2 == _TRUE_)),
               errmsg,
               "In input file, you can only enter one of z_reio or tau_reio, choose one");
    if (flag1 == _TRUE_) {
      pth->z_reio=param1;
      pth->reio_z_or_tau=reio_z;
    }
    if (flag2 == _TRUE_) {
      pth->tau_reio=param2;
      pth->reio_z_or_tau=reio_tau;
    }

    class_read_double("reionization_exponent",pth->reionization_exponent);
    class_read_double("reionization_width",pth->reionization_width);
    class_read_double("helium_fullreio_redshift",pth->helium_fullreio_redshift);
    class_read_double("helium_fullreio_width",pth->helium_fullreio_width);

  }

  /* reionization parameters if reio_parametrization=reio_bins_tanh */
  if (pth->reio_parametrization == reio_bins_tanh) {
    class_read_int("binned_reio_num",pth->binned_reio_num);
    class_read_list_of_doubles("binned_reio_z",pth->binned_reio_z,pth->binned_reio_num);
    class_read_list_of_doubles("binned_reio_xe",pth->binned_reio_xe,pth->binned_reio_num);
    class_read_double("binned_reio_step_sharpness",pth->binned_reio_step_sharpness);
  }

  /* energy injection parameters from CDM annihilation/decay */
  class_read_double("annihilation",pth->annihilation);
  class_read_double("decay",pth->decay);
  class_read_double("annihilation_variation",pth->annihilation_variation);
  class_read_double("annihilation_z",pth->annihilation_z);
  class_read_double("annihilation_zmax",pth->annihilation_zmax);
  class_read_double("annihilation_zmin",pth->annihilation_zmin);
  class_read_double("annihilation_f_halo",pth->annihilation_f_halo);
  class_read_double("annihilation_z_halo",pth->annihilation_z_halo);

  class_call(parser_read_string(pfc,
                                "on the spot",
                                &(string1),
                                &(flag1),
                                errmsg),
             errmsg,
             errmsg);

  if ((flag1 == _TRUE_) && ((strstr(string1,"y") != NULL) || (strstr(string1,"Y") != NULL))) {
    pth->has_on_the_spot = _TRUE_;
  }
  else {
    pth->has_on_the_spot = _FALSE_;
  }

  /** (c) define which perturbations and sources should be computed, and down to which scale */

  ppt->has_perturbations = _FALSE_;
  ppt->has_cls = _FALSE_;

  class_call(parser_read_string(pfc,"output",&string1,&flag1,errmsg),
             errmsg,
             errmsg);

  if (flag1 == _TRUE_) {

    if ((strstr(string1,"tCl") != NULL) || (strstr(string1,"TCl") != NULL) || (strstr(string1,"TCL") != NULL)) {
      ppt->has_cl_cmb_temperature = _TRUE_;  
      ppt->has_perturbations = _TRUE_;  
      ppt->has_cls = _TRUE_;
    }

    if ((strstr(string1,"pCl") != NULL) || (strstr(string1,"PCl") != NULL) || (strstr(string1,"PCL") != NULL)) {
      ppt->has_cl_cmb_polarization = _TRUE_;  
      ppt->has_perturbations = _TRUE_;  
      ppt->has_cls = _TRUE_;
    }
    
    if ((strstr(string1,"lCl") != NULL) || (strstr(string1,"LCl") != NULL) || (strstr(string1,"LCL") != NULL)) {
      ppt->has_cl_cmb_lensing_potential = _TRUE_;
      ppt->has_perturbations = _TRUE_; 
      ppt->has_cls = _TRUE_;
    }

    if ((strstr(string1,"dCl") != NULL) || (strstr(string1,"DCl") != NULL) || (strstr(string1,"DCL") != NULL)) {
      ppt->has_cl_density=_TRUE_;
      ppt->has_perturbations = _TRUE_;
      ppt->has_cls = _TRUE_;
    }

    if ((strstr(string1,"sCl") != NULL) || (strstr(string1,"SCl") != NULL) || (strstr(string1,"SCL") != NULL)) {
      ppt->has_cl_lensing_potential=_TRUE_;
      ppt->has_perturbations = _TRUE_;
      ppt->has_cls = _TRUE_;
    }

    if ((strstr(string1,"mPk") != NULL) || (strstr(string1,"MPk") != NULL) || (strstr(string1,"MPK") != NULL)) {
      ppt->has_pk_matter=_TRUE_; 
      ppt->has_perturbations = _TRUE_;  
    }

    if ((strstr(string1,"mTk") != NULL) || (strstr(string1,"MTk") != NULL) || (strstr(string1,"MTK") != NULL) ||
        (strstr(string1,"dTk") != NULL) || (strstr(string1,"DTk") != NULL) || (strstr(string1,"DTK") != NULL)) {
      ppt->has_density_transfers=_TRUE_; 
      ppt->has_perturbations = _TRUE_;  
    }

    if ((strstr(string1,"vTk") != NULL) || (strstr(string1,"VTk") != NULL) || (strstr(string1,"VTK") != NULL)) {
      ppt->has_velocity_transfers=_TRUE_; 
      ppt->has_perturbations = _TRUE_;  
    }

  }

  if (ppt->has_perturbations == _TRUE_) { 

    class_call(parser_read_string(pfc,"modes",&string1,&flag1,errmsg),
               errmsg,
               errmsg);

    if (flag1 == _TRUE_) {

      /* if no modes are specified, the default is has_scalars=_TRUE_; 
         but if they are specified we should reset has_scalars to _FALSE_ before reading */
      ppt->has_scalars=_FALSE_;

      if ((strstr(string1,"s") != NULL) || (strstr(string1,"S") != NULL))
        ppt->has_scalars=_TRUE_; 

      if ((strstr(string1,"v") != NULL) || (strstr(string1,"V") != NULL))
        ppt->has_vectors=_TRUE_;  

      if ((strstr(string1,"t") != NULL) || (strstr(string1,"T") != NULL))
        ppt->has_tensors=_TRUE_;

      class_test(class_none_of_three(ppt->has_scalars,ppt->has_vectors,ppt->has_tensors),
                 errmsg,	       
                 "You wrote: modes=%s. Could not identify any of the modes ('s', 'v', 't') in such input",string1);
    }


    class_read_double("alpha",ppt->alpha);
    class_read_double("beta",ppt->beta);
    class_read_double("lambda",ppt->lambda);
    class_read_double("Y_dm",ppt->Y_dm);

    if (ppt->has_scalars == _TRUE_) {

      class_call(parser_read_string(pfc,"ic",&string1,&flag1,errmsg),
                 errmsg,
                 errmsg);

      if (flag1 == _TRUE_) {

        /* if no initial conditions are specified, the default is has_ad=_TRUE_; 
           but if they are specified we should reset has_ad to _FALSE_ before reading */
        ppt->has_ad=_FALSE_;

        if ((strstr(string1,"ad") != NULL) || (strstr(string1,"AD") != NULL))
          ppt->has_ad=_TRUE_; 
	
        if ((strstr(string1,"bi") != NULL) || (strstr(string1,"BI") != NULL))
          ppt->has_bi=_TRUE_; 
	
        if ((strstr(string1,"cdi") != NULL) || (strstr(string1,"CDI") != NULL))
          ppt->has_cdi=_TRUE_; 
	
        if ((strstr(string1,"nid") != NULL) || (strstr(string1,"NID") != NULL))
          ppt->has_nid=_TRUE_; 
	
        if ((strstr(string1,"niv") != NULL) || (strstr(string1,"NIV") != NULL))
          ppt->has_niv=_TRUE_; 
      
        class_test(ppt->has_ad==_FALSE_ && ppt->has_bi ==_FALSE_ && ppt->has_cdi ==_FALSE_ && ppt->has_nid ==_FALSE_ && ppt->has_niv ==_FALSE_,
                   errmsg,	       
                   "You wrote: ic=%s. Could not identify any of the initial conditions ('ad', 'bi', 'cdi', 'nid', 'niv') in such input",string1);
	
      }
    }

    else {

      class_test(ppt->has_cl_cmb_lensing_potential == _TRUE_,
                 errmsg,
                 "Inconsistency: you want C_l's for cmb lensing potential, but no scalar modes\n");

      class_test(ppt->has_pk_matter == _TRUE_,
                 errmsg,
                 "Inconsistency: you want P(k) of matter, but no scalar modes\n");

    }

    class_call(parser_read_string(pfc,"gauge",&string1,&flag1,errmsg),
               errmsg,
               errmsg);
    
    if (flag1 == _TRUE_) {
      
      if ((strstr(string1,"newtonian") != NULL) || (strstr(string1,"Newtonian") != NULL) || (strstr(string1,"new") != NULL)) {
        ppt->gauge = 0;
      }
      
      if ((strstr(string1,"synchronous") != NULL) || (strstr(string1,"sync") != NULL) || (strstr(string1,"Synchronous") != NULL)) {
        ppt->gauge = 1;
      }
    }
    
  }

  /** (d) define the primordial spectrum */

  class_call(parser_read_string(pfc,"P_k_ini type",&string1,&flag1,errmsg),
             errmsg,
             errmsg);

  if (flag1 == _TRUE_) {
    flag2=_FALSE_;
    if (strcmp(string1,"analytic_Pk") == 0) {
      ppm->primordial_spec_type = analytic_Pk;
      flag2=_TRUE_;
    }
    if (strcmp(string1,"two_scales") == 0) {
      ppm->primordial_spec_type = two_scales;
      flag2=_TRUE_;
    }
    if (strcmp(string1,"inflation_V") == 0) {
      ppm->primordial_spec_type = inflation_V;
      flag2=_TRUE_;
    }
    class_test(flag2==_FALSE_,
               errmsg,
               "could not identify primordial spectrum type, check that it is one of 'analytic_pk', 'inflation_V'...");
  }

  class_read_double("k_pivot",ppm->k_pivot);

  if (ppm->primordial_spec_type == two_scales) {

    class_read_double("k1",k1);
    class_read_double("k2",k2);
    class_test(k1<=0.,errmsg,"enter strictly positive scale k1");
    class_test(k2<=0.,errmsg,"enter strictly positive scale k2");
	
    if (ppt->has_scalars == _TRUE_) {

      class_read_double("P_{RR}^1",prr1);
      class_read_double("P_{RR}^2",prr2);
      class_test(prr1<=0.,errmsg,"enter strictly positive scale P_{RR}^1");
      class_test(prr2<=0.,errmsg,"enter strictly positive scale P_{RR}^2");

      ppm->n_s = log(prr2/prr1)/log(k2/k1)+1.;
      ppm->A_s = prr1*exp((ppm->n_s-1.)*log(ppm->k_pivot/k1));

      if ((ppt->has_bi == _TRUE_) ||
          (ppt->has_cdi == _TRUE_) ||
          (ppt->has_nid == _TRUE_) ||
          (ppt->has_niv == _TRUE_)) {

        class_read_double("P_{II}^1",pii1);
        class_read_double("P_{II}^2",pii2);
        class_read_double("P_{RI}^1",pri1);
        class_read_double("|P_{RI}^2|",pri2);
	
        class_test(pii1 <= 0.,
                   errmsg,
                   "since you request iso modes, you should have P_{ii}^1 strictly positive");
        class_test(pii2 < 0.,
                   errmsg,
                   "since you request iso modes, you should have P_{ii}^2 positive or eventually null");
        class_test(pri2 < 0.,
                   errmsg,
                   "by definition, you should have |P_{ri}^2| positive or eventually null");

        flag1 = _FALSE_;

        class_call(parser_read_string(pfc,"special iso",&string1,&flag1,errmsg),
                   errmsg,
                   errmsg);
	
        /* axion case, only one iso parameter: piir1  */
        if ((flag1 == _TRUE_) && (strstr(string1,"axion") != NULL)) {
          n_iso = 1.;
          n_cor = 0.;
          c_cor = 0.;
        }
        /* curvaton case, only one iso parameter: piir1  */
        else if ((flag1 == _TRUE_) && (strstr(string1,"anticurvaton") != NULL)) {
          n_iso = ppm->n_s;
          n_cor = 0.;
          c_cor = 1.;
        }
        /* inverted-correlation-curvaton case, only one iso parameter: piir1  */
        else if ((flag1 == _TRUE_) && (strstr(string1,"curvaton") != NULL)) {
          n_iso = ppm->n_s;
          n_cor = 0.;
          c_cor = -1.;
        }
        /* general case, but if pii2 or pri2=0 the code interprets it
           as a request for n_iso=n_ad or n_cor=0 respectively */
        else {
          if (pii2 == 0.) {
            n_iso = ppm->n_s;
          }
          else {
            class_test((pii1==0.) || (pii2 == 0.) || (pii1*pii2<0.),errmsg,"should NEVER happen"); 
            n_iso = log(pii2/pii1)/log(k2/k1)+1.;
          }
          class_test(pri1==0,errmsg,"the general isocurvature case requires a non-zero P_{RI}^1");
          if (pri2 == 0.) {
            n_cor = 0.;
          }
          else {
            class_test((pri1==0.) || (pri2 <= 0.) || (pii1*pii2<0),errmsg,"should NEVER happen");
            n_cor = log(pri2/fabs(pri1))/log(k2/k1)-0.5*(ppm->n_s+n_iso-2.);
          }
          class_test((pii1*prr1<=0.),errmsg,"should NEVER happen");
          class_test(fabs(pri1)/sqrt(pii1*prr1)>1,errmsg,"too large ad-iso cross-correlation in k1");
          class_test(fabs(pri1)/sqrt(pii1*prr1)*exp(n_cor*log(k2/k1))>1,errmsg,"too large ad-iso cross-correlation in k2");
          c_cor = -pri1/sqrt(pii1*prr1)*exp(n_cor*log(ppm->k_pivot/k1));
        }
        /* formula for f_iso valid in all cases */
        class_test((pii1==0.) || (prr1 == 0.) || (pii1*prr1<0.),errmsg,"should NEVER happen");
        f_iso = sqrt(pii1/prr1)*exp(0.5*(n_iso-ppm->n_s)*log(ppm->k_pivot/k1));
        
      }
      
      if (ppt->has_bi == _TRUE_) {
        ppm->f_bi = f_iso;
        ppm->n_bi = n_iso;
        ppm->c_ad_bi = c_cor;
        ppm->n_ad_bi = n_cor;
      }
      
      if (ppt->has_cdi == _TRUE_) {
        ppm->f_cdi = f_iso;
        ppm->n_cdi = n_iso;
        ppm->c_ad_cdi = c_cor;
        ppm->n_ad_cdi = n_cor;
      }
      
      if (ppt->has_nid == _TRUE_) {
        ppm->f_nid = f_iso;
        ppm->n_nid = n_iso;
        ppm->c_ad_nid = c_cor;
        ppm->n_ad_nid = n_cor;
      }
      
      if (ppt->has_niv == _TRUE_) {
        ppm->f_niv = f_iso;
        ppm->n_niv = n_iso;
        ppm->c_ad_niv = c_cor;
        ppm->n_ad_niv = n_cor;
      }
    }

    ppm->primordial_spec_type = analytic_Pk;

  }

  else if (ppm->primordial_spec_type == analytic_Pk) {

    if (ppt->has_scalars == _TRUE_) {
      
      class_call(parser_read_double(pfc,"A_s",&param1,&flag1,errmsg),
                 errmsg,
                 errmsg);
      class_call(parser_read_double(pfc,"ln10^{10}A_s",&param2,&flag2,errmsg),
                 errmsg,
                 errmsg);
      class_test((flag1 == _TRUE_) && (flag2 == _TRUE_),
                 errmsg,
                 "In input file, you cannot enter both A_s and ln10^{10}A_s, choose one");
      if (flag1 == _TRUE_)
        ppm->A_s = param1;
      else
        ppm->A_s = exp(param2)*1.e-10;

      if (ppt->has_ad == _TRUE_) {

        class_read_double("n_s",ppm->n_s);
        class_read_double("alpha_s",ppm->alpha_s);

      }

      if (ppt->has_bi == _TRUE_) {

        class_read_double("f_bi",ppm->f_bi);
        class_read_double("n_bi",ppm->n_bi);
        class_read_double("alpha_bi",ppm->alpha_bi);

      }

      if (ppt->has_cdi == _TRUE_) {

        class_read_double("f_cdi",ppm->f_cdi);
        class_read_double("n_cdi",ppm->n_cdi);
        class_read_double("alpha_cdi",ppm->alpha_cdi);

      }

      if (ppt->has_nid == _TRUE_) {

        class_read_double("f_nid",ppm->f_nid);
        class_read_double("n_nid",ppm->n_nid);
        class_read_double("alpha_nid",ppm->alpha_nid);

      }

      if (ppt->has_niv == _TRUE_) {

        class_read_double("f_niv",ppm->f_niv);
        class_read_double("n_niv",ppm->n_niv);
        class_read_double("alpha_niv",ppm->alpha_niv);

      }

      if ((ppt->has_ad == _TRUE_) && (ppt->has_bi == _TRUE_)) {
        class_read_double_one_of_two("c_ad_bi","c_bi_ad",ppm->c_ad_bi);
        class_read_double_one_of_two("n_ad_bi","n_bi_ad",ppm->n_ad_bi);
        class_read_double_one_of_two("alpha_ad_bi","alpha_bi_ad",ppm->alpha_ad_bi);
      }

      if ((ppt->has_ad == _TRUE_) && (ppt->has_cdi == _TRUE_)) {
        class_read_double_one_of_two("c_ad_cdi","c_cdi_ad",ppm->c_ad_cdi);
        class_read_double_one_of_two("n_ad_cdi","n_cdi_ad",ppm->n_ad_cdi);
        class_read_double_one_of_two("alpha_ad_cdi","alpha_cdi_ad",ppm->alpha_ad_cdi);
      }

      if ((ppt->has_ad == _TRUE_) && (ppt->has_nid == _TRUE_)) {
        class_read_double_one_of_two("c_ad_nid","c_nid_ad",ppm->c_ad_nid);
        class_read_double_one_of_two("n_ad_nid","n_nid_ad",ppm->n_ad_nid);
        class_read_double_one_of_two("alpha_ad_nid","alpha_nid_ad",ppm->alpha_ad_nid);
      }

      if ((ppt->has_ad == _TRUE_) && (ppt->has_niv == _TRUE_)) {
        class_read_double_one_of_two("c_ad_niv","c_niv_ad",ppm->c_ad_niv);
        class_read_double_one_of_two("n_ad_niv","n_niv_ad",ppm->n_ad_niv);
        class_read_double_one_of_two("alpha_ad_niv","alpha_niv_ad",ppm->alpha_ad_niv);
      }

      if ((ppt->has_bi == _TRUE_) && (ppt->has_cdi == _TRUE_)) {
        class_read_double_one_of_two("c_bi_cdi","c_cdi_bi",ppm->c_bi_cdi);
        class_read_double_one_of_two("n_bi_cdi","n_cdi_bi",ppm->n_bi_cdi);
        class_read_double_one_of_two("alpha_bi_cdi","alpha_cdi_bi",ppm->alpha_bi_cdi);
      }

      if ((ppt->has_bi == _TRUE_) && (ppt->has_nid == _TRUE_)) {
        class_read_double_one_of_two("c_bi_nid","c_nid_bi",ppm->c_bi_nid);
        class_read_double_one_of_two("n_bi_nid","n_nid_bi",ppm->n_bi_nid);
        class_read_double_one_of_two("alpha_bi_nid","alpha_nid_bi",ppm->alpha_bi_nid);
      }

      if ((ppt->has_bi == _TRUE_) && (ppt->has_niv == _TRUE_)) {
        class_read_double_one_of_two("c_bi_niv","c_niv_bi",ppm->c_bi_niv);
        class_read_double_one_of_two("n_bi_niv","n_niv_bi",ppm->n_bi_niv);
        class_read_double_one_of_two("alpha_bi_niv","alpha_niv_bi",ppm->alpha_bi_niv);
      }

      if ((ppt->has_cdi == _TRUE_) && (ppt->has_nid == _TRUE_)) {
        class_read_double_one_of_two("c_cdi_nid","c_nid_cdi",ppm->c_cdi_nid);
        class_read_double_one_of_two("n_cdi_nid","n_nid_cdi",ppm->n_cdi_nid);
        class_read_double_one_of_two("alpha_cdi_nid","alpha_nid_cdi",ppm->alpha_cdi_nid);
      }

      if ((ppt->has_cdi == _TRUE_) && (ppt->has_niv == _TRUE_)) {
        class_read_double_one_of_two("c_cdi_niv","c_niv_cdi",ppm->c_cdi_niv);
        class_read_double_one_of_two("n_cdi_niv","n_niv_cdi",ppm->n_cdi_niv);
        class_read_double_one_of_two("alpha_cdi_niv","alpha_niv_cdi",ppm->alpha_cdi_niv);
      }

      if ((ppt->has_nid == _TRUE_) && (ppt->has_niv == _TRUE_)) {
        class_read_double_one_of_two("c_nid_niv","c_niv_nid",ppm->c_nid_niv);
        class_read_double_one_of_two("n_nid_niv","n_niv_nid",ppm->n_nid_niv);
        class_read_double_one_of_two("alpha_nid_niv","alpha_niv_nid",ppm->alpha_nid_niv);
      }  

    }

    if (ppt->has_tensors == _TRUE_) {
    
      class_read_double("r",ppm->r);

      class_call(parser_read_string(pfc,"n_t",&string1,&flag1,errmsg),
                 errmsg,
                 errmsg);

      if (flag1 == _TRUE_) {
    
        if ((strstr(string1,"SCC") != NULL) || (strstr(string1,"scc") != NULL)) {
          ppm->n_t = -ppm->r/8.*(2.-ppm->r/8.-ppm->n_s);  
        }
        else {
          class_read_double("n_t",ppm->n_t);
        }
    
      }

      class_call(parser_read_string(pfc,"alpha_t",&string1,&flag1,errmsg),
                 errmsg,
                 errmsg);

      if (flag1 == _TRUE_) {
    
        if ((strstr(string1,"SCC") != NULL) || (strstr(string1,"scc") != NULL)) {
          ppm->alpha_t = ppm->r/8.*(ppm->r/8.+ppm->n_s-1.);  
        }
        else {
          class_read_double("alpha_t",ppm->alpha_t);
        }
    
      }
    }
  }

  else if (ppm->primordial_spec_type == inflation_V) {

    class_call(parser_read_string(pfc,"potential",&string1,&flag1,errmsg),
               errmsg,
               errmsg);
    /** only polynomial coded so far: no need to interpret string1 **/		 

    class_read_double("phi_pivot",ppm->phi_pivot);

    class_call(parser_read_string(pfc,"R_0",&string1,&flag1,errmsg),
               errmsg,
               errmsg);
    
    if (flag1 == _TRUE_) {

      R0=0.;
      R1=0.;
      R2=0.;
      R3=0.;
      R4=0.;

      class_read_double("R_0",R0);
      class_read_double("R_1",R1);
      class_read_double("R_2",R2);
      class_read_double("R_3",R3);
      class_read_double("R_4",R4);

      class_test(R0 <= 0.,
                 errmsg,
                 "inconsistent parametrisation of polynomial inflation potential");
      class_test(R1 <= 0.,
                 errmsg,
                 "inconsistent parametrisation of polynomial inflation potential");

      ppm->V0 = R0*R1*3./128./_PI_;
      ppm->V1 = -sqrt(R1)*ppm->V0;
      ppm->V2 = R2*ppm->V0;
      ppm->V3 = R3*ppm->V0*ppm->V0/ppm->V1;
      ppm->V4 = R4*ppm->V0/R1;
    }
    else {
      class_read_double("V_0",ppm->V0);
      class_read_double("V_1",ppm->V1);
      class_read_double("V_2",ppm->V2);
      class_read_double("V_3",ppm->V3);
      class_read_double("V_4",ppm->V4);
    }

  }

  /** (e) parameters for final spectra */

  if (ppt->has_cls == _TRUE_) {

    if (ppt->has_scalars == _TRUE_) {
      if ((ppt->has_cl_cmb_temperature == _TRUE_) || 
          (ppt->has_cl_cmb_polarization == _TRUE_) || 
          (ppt->has_cl_cmb_lensing_potential == _TRUE_)) 
        class_read_double("l_max_scalars",ppt->l_scalar_max);

      if ((ppt->has_cl_lensing_potential == _TRUE_) || (ppt->has_cl_density == _TRUE_))
        class_read_double("l_max_lss",ppt->l_lss_max);
    }

    if (ppt->has_tensors == _TRUE_) {   
      class_read_double("l_max_tensors",ppt->l_tensor_max);
    }
  }

  if ((ppt->has_scalars == _TRUE_) && 
      ((ppt->has_cl_cmb_temperature == _TRUE_) || (ppt->has_cl_cmb_polarization == _TRUE_)) && 
      (ppt->has_cl_cmb_lensing_potential == _TRUE_)) {
    
    class_call(parser_read_string(pfc,
                                  "lensing",
                                  &(string1),
                                  &(flag1),
                                  errmsg),
               errmsg,
               errmsg);
    
    if ((flag1 == _TRUE_) && ((strstr(string1,"y") != NULL) || (strstr(string1,"Y") != NULL))) {
      ple->has_lensed_cls = _TRUE_;
    }

    class_read_double("lcmb_rescale",ptr->lcmb_rescale);
    class_read_double("lcmb_tilt",ptr->lcmb_tilt);
    class_read_double("lcmb_pivot",ptr->lcmb_pivot);

  }

  if ((ppt->has_pk_matter == _TRUE_) || (ppt->has_density_transfers == _TRUE_) || (ppt->has_velocity_transfers == _TRUE_)) {

    class_call(parser_read_double(pfc,"P_k_max_h/Mpc",&param1,&flag1,errmsg),
               errmsg,
               errmsg);
    class_call(parser_read_double(pfc,"P_k_max_1/Mpc",&param2,&flag2,errmsg),
               errmsg,
               errmsg);
    class_test((flag1 == _TRUE_) && (flag2 == _TRUE_),
               errmsg,
               "In input file, you cannot enter both P_k_max_h/Mpc and P_k_max_1/Mpc, choose one");
    if (flag1 == _TRUE_) {
      ppt->k_scalar_kmax_for_pk=param1*pba->h;
    }
    if (flag2 == _TRUE_) {
      ppt->k_scalar_kmax_for_pk=param2;
    }

    class_call(parser_read_list_of_doubles(pfc,
                                           "z_pk",
                                           &(int1),
                                           &(pointer1),
                                           &flag1,
                                           errmsg),
               errmsg,
               errmsg);
    
    if (flag1 == _TRUE_) {
      class_test(int1 > _Z_PK_NUM_MAX_,
                 errmsg,
                 "you want to write some output for %d different values of z, hence you should increase _Z_PK_NUM_MAX_ in include/output.h to at least this number",
                 int1);
      pop->z_pk_num = int1;
      for (i=0; i<int1; i++) {
        pop->z_pk[i] = pointer1[i];
      }
      free(pointer1);
    }
    
    class_call(parser_read_double(pfc,"z_max_pk",&param1,&flag1,errmsg),
               errmsg,
               errmsg);
  
    if (flag1==_TRUE_) {
      psp->z_max_pk = param1;
    }
    else {
      psp->z_max_pk = 0.;
      for (i=0; i<pop->z_pk_num; i++)
        psp->z_max_pk = max(psp->z_max_pk,pop->z_pk[i]);
    }
  }

  /* deal with selection functions */
  if ((ppt->has_cl_density == _TRUE_) || (ppt->has_cl_lensing_potential == _TRUE_)) {
    
    class_call(parser_read_string(pfc,
                                  "selection",
                                  &(string1),
                                  &(flag1),
                                  errmsg),
               errmsg,
               errmsg);
    
    if (flag1 == _TRUE_) {
      if (strstr(string1,"gaussian") != NULL) {
        ppt->selection=gaussian; 
      }
      else if (strstr(string1,"tophat") != NULL) {
        ppt->selection=tophat; 
      }
      else if (strstr(string1,"dirac") != NULL) {
        ppt->selection=dirac; 
      }
      else {
        class_stop(errmsg,"In selection function input: type %s is unclear",string1);
      }
    }
    
    class_call(parser_read_list_of_doubles(pfc,
                                           "selection_mean",
                                           &(int1),
                                           &(pointer1),
                                           &flag1,
                                           errmsg),
               errmsg,
               errmsg);
    
    if ((flag1 == _TRUE_) && (int1>0)) {

      class_test(int1 > _SELECTION_NUM_MAX_,
                 errmsg,
                 "you want to compute density Cl's for %d different bins, hence you should increase _SELECTION_NUM_MAX_ in include/transfer.h to at least this number",
                 int1);

      ppt->selection_num = int1;
      for (i=0; i<int1; i++) {
        class_test((pointer1[i] < 0.) || (pointer1[i] > 1000.),
                   errmsg,
                   "input of selection functions: you asked for a mean redshift equal to %e, sounds odd",
                   pointer1[i]);
        ppt->selection_mean[i] = pointer1[i];
      }
      free(pointer1);
      /* first set all widths to default; correct eventually later */
      for (i=1; i<int1; i++) {
        class_test(ppt->selection_mean[i]<=ppt->selection_mean[i-1],
                   errmsg,
                   "input of selection functions: the list of mean redshifts must be passed in growing order; you entered %e before %e",ppt->selection_mean[i-1],ppt->selection_mean[i]);
        ppt->selection_width[i] = ppt->selection_width[0];
      }

      class_call(parser_read_list_of_doubles(pfc,
                                             "selection_width",
                                             &(int1),
                                             &(pointer1),
                                             &flag1,
                                             errmsg),
                 errmsg,
                 errmsg);
    
      if ((flag1 == _TRUE_) && (int1>0)) {
	
        if (int1==1) {
          for (i=0; i<ppt->selection_num; i++) {
            ppt->selection_width[i] = pointer1[0];
          }	
        }
        else if (int1==ppt->selection_num) {
          for (i=0; i<int1; i++) {
            ppt->selection_width[i] = pointer1[i];
          }	
        }
        else {
          class_stop(errmsg,
                     "In input for selection function, you asked for %d bin centers and %d bin widths; number of bins unclear; you should pass either one bin width (common to all bins) or %d bin witdths",
                     ppt->selection_num,int1,ppt->selection_num);
        }
        free(pointer1);
      }
    }
  
    if (ppt->selection_num>1) {
      class_read_int("non_diagonal",psp->non_diag);
      if ((psp->non_diag<0) || (psp->non_diag>=ppt->selection_num))
        class_stop(errmsg,
                   "Input for non_diagonal is %d, while it is expected to be between 0 and %d\n",
                   psp->non_diag,ppt->selection_num-1);
    }
  }

  class_read_string("root",pop->root);

  class_call(parser_read_string(pfc,
                                "headers",
                                &(string1),
                                &(flag1),
                                errmsg),
             errmsg,
             errmsg);
	     
  if ((flag1 == _TRUE_) && ((strstr(string1,"y") == NULL) && (strstr(string1,"Y") == NULL))) {
    pop->write_header = _FALSE_;
  }

  class_call(parser_read_string(pfc,"format",&string1,&flag1,errmsg),
             errmsg,
             errmsg);

  if (flag1 == _TRUE_) {

    if ((strstr(string1,"class") != NULL) || (strstr(string1,"CLASS") != NULL))
      pop->output_format = class_format;
    else {
      if ((strstr(string1,"camb") != NULL) || (strstr(string1,"CAMB") != NULL))
        pop->output_format = camb_format;
      else
        class_stop(errmsg,	       
                   "You wrote: format=%s. Could not identify any of the possible formats ('class', 'CLASS', 'camb', 'CAMB')",string1);	  
    }
  }
  
  class_call(parser_read_string(pfc,
                                "bessel file",
                                &(string1),
                                &(flag1),
                                errmsg),
             errmsg,
             errmsg);
	     
  if ((flag1 == _TRUE_) && ((strstr(string1,"y") != NULL) || (strstr(string1,"Y") != NULL))) {
    pbs->bessel_always_recompute = _FALSE_;
  }

  /** (f) parameter related to the non-linear spectra computation */

  class_call(parser_read_string(pfc,
                                "non linear",
                                &(string1),
                                &(flag1),
                                errmsg),
             errmsg,
             errmsg);

  if (flag1 == _TRUE_) {

    if ((strstr(string1,"halofit") != NULL) || (strstr(string1,"Halofit") != NULL) || (strstr(string1,"HALOFIT") != NULL)) {
      pnl->method=nl_halofit;
    }
    if ((strstr(string1,"trg") != NULL) || (strstr(string1,"TRG") != NULL)) {
      pnl->method=nl_trg;
    }    
    if ((strstr(string1,"one-loop") != NULL) || (strstr(string1,"oneloop") != NULL) || (strstr(string1,"one loop") != NULL)) {
      pnl->method=nl_trg_one_loop;
    }
    if ((strstr(string1,"test linear") != NULL) || (strstr(string1,"test-linear") != NULL)) {
      pnl->method=nl_trg_linear;
    }

    class_test((pnl->method>nl_none) && (ppt->has_pk_matter==_FALSE_),
               errmsg,
               "it is not consistent to ask for non-linear power spectrum but not for linear one: you should include mPk in the 'output' entry");
        
    if (pnl->method==nl_trg) {
      
      class_call(parser_read_string(pfc,
                                    "non linear ic",
                                    &(string1),
                                    &(flag1),
                                    errmsg),
                 errmsg,
                 errmsg);
      
      if ((strstr(string1,"linear") != NULL) || (strstr(string1,"lin") != NULL)) {
        pnl->ic=nl_lin;
      }
    }
  }

  /** (g) amount of information sent to standard output (none if all set to zero) */

  class_read_int("background_verbose",
                 pba->background_verbose);

  class_read_int("thermodynamics_verbose",
                 pth->thermodynamics_verbose);

  class_read_int("perturbations_verbose",
                 ppt->perturbations_verbose);

  class_read_int("bessels_verbose",
                 pbs->bessels_verbose);

  class_read_int("transfer_verbose",
                 ptr->transfer_verbose);

  class_read_int("primordial_verbose",
                 ppm->primordial_verbose);

  class_read_int("spectra_verbose",
                 psp->spectra_verbose);

  class_read_int("nonlinear_verbose",
                 pnl->nonlinear_verbose);

  class_read_int("lensing_verbose",
                 ple->lensing_verbose);

  class_read_int("output_verbose",
                 pop->output_verbose);

  /** (h) all precision parameters */

  /** h.1. parameters related to the background */

  class_read_double("a_ini_over_a_today_default",ppr->a_ini_over_a_today_default);
  class_read_double("back_integration_stepsize",ppr->back_integration_stepsize);
  class_read_double("tol_background_integration",ppr->tol_background_integration);
  class_read_double("tol_initial_Omega_r",ppr->tol_initial_Omega_r);
  class_read_double("tol_ncdm_initial_w",ppr->tol_ncdm_initial_w);

  /** h.2. parameters related to the thermodynamics */

  class_read_string("sBBN file",ppr->sBBN_file);

  class_read_double("recfast_z_initial",ppr->recfast_z_initial);

  class_read_int("recfast_Nz0",ppr->recfast_Nz0);
  class_read_double("tol_thermo_integration",ppr->tol_thermo_integration);

  class_read_int("recfast_Heswitch",ppr->recfast_Heswitch);
  class_read_double("recfast_fudge_He",ppr->recfast_fudge_He);

  class_read_int("recfast_Hswitch",ppr->recfast_Hswitch);
  class_read_double("recfast_fudge_H",ppr->recfast_fudge_H);
  if (ppr->recfast_Hswitch == _TRUE_) {
    class_read_double("recfast_delta_fudge_H",ppr->recfast_delta_fudge_H);
    class_read_double("recfast_AGauss1",ppr->recfast_AGauss1);
    class_read_double("recfast_AGauss2",ppr->recfast_AGauss2);
    class_read_double("recfast_zGauss1",ppr->recfast_zGauss1);
    class_read_double("recfast_zGauss2",ppr->recfast_zGauss2);
    class_read_double("recfast_wGauss1",ppr->recfast_wGauss1);
    class_read_double("recfast_wGauss2",ppr->recfast_wGauss2);
  }

  class_read_double("recfast_z_He_1",ppr->recfast_z_He_1);
  class_read_double("recfast_delta_z_He_1",ppr->recfast_delta_z_He_1);
  class_read_double("recfast_z_He_2",ppr->recfast_z_He_2);
  class_read_double("recfast_delta_z_He_2",ppr->recfast_delta_z_He_2);
  class_read_double("recfast_z_He_3",ppr->recfast_z_He_3);
  class_read_double("recfast_delta_z_He_3",ppr->recfast_delta_z_He_3);
  class_read_double("recfast_x_He0_trigger",ppr->recfast_x_He0_trigger);
  class_read_double("recfast_x_He0_trigger2",ppr->recfast_x_He0_trigger2);
  class_read_double("recfast_x_He0_trigger_delta",ppr->recfast_x_He0_trigger_delta);
  class_read_double("recfast_x_H0_trigger",ppr->recfast_x_H0_trigger);
  class_read_double("recfast_x_H0_trigger2",ppr->recfast_x_H0_trigger2);
  class_read_double("recfast_x_H0_trigger_delta",ppr->recfast_x_H0_trigger_delta);
  class_read_double("recfast_H_frac",ppr->recfast_H_frac);

  class_read_string("Alpha_inf hyrec file",ppr->hyrec_Alpha_inf_file);
  class_read_string("R_inf hyrec file",ppr->hyrec_R_inf_file);
  class_read_string("two_photon_tables hyrec file",ppr->hyrec_two_photon_tables_file);

  class_read_double("reionization_z_start_max",ppr->reionization_z_start_max);
  class_read_double("reionization_sampling",ppr->reionization_sampling);
  class_read_double("reionization_optical_depth_tol",ppr->reionization_optical_depth_tol);
  class_read_double("reionization_start_factor",ppr->reionization_start_factor);

  class_read_int("thermo_rate_smoothing_radius",ppr->thermo_rate_smoothing_radius);

  /** h.3. parameters related to the perturbations */

  class_read_int("evolver",ppr->evolver);
  class_read_int("pk_definition",ppr->pk_definition);
  class_read_double("k_scalar_min_tau0",ppr->k_scalar_min_tau0);
  class_read_double("k_scalar_max_tau0_over_l_max",ppr->k_scalar_max_tau0_over_l_max);
  class_read_double("k_scalar_step_sub",ppr->k_scalar_step_sub);
  class_read_double("k_scalar_step_super",ppr->k_scalar_step_super);
  class_read_double("k_scalar_step_transition",ppr->k_scalar_step_transition);
  class_read_double("k_scalar_k_per_decade_for_pk",ppr->k_scalar_k_per_decade_for_pk);
  class_read_double("k_scalar_k_per_decade_for_bao",ppr->k_scalar_k_per_decade_for_bao);
  class_read_double("k_scalar_bao_center",ppr->k_scalar_bao_center);
  class_read_double("k_scalar_bao_width",ppr->k_scalar_bao_width);
  class_read_double("k_tensor_min_tau0",ppr->k_tensor_min_tau0);
  class_read_double("k_tensor_max_tau0_over_l_max",ppr->k_tensor_max_tau0_over_l_max);
  class_read_double("k_tensor_step_sub",ppr->k_tensor_step_sub);
  class_read_double("k_tensor_step_super",ppr->k_tensor_step_super);
  class_read_double("k_tensor_step_transition",ppr->k_tensor_step_transition);
  class_read_double("start_small_k_at_tau_c_over_tau_h",ppr->start_small_k_at_tau_c_over_tau_h);
  class_read_double("start_large_k_at_tau_h_over_tau_k",ppr->start_large_k_at_tau_h_over_tau_k);
  class_read_double("tight_coupling_trigger_tau_c_over_tau_h",ppr->tight_coupling_trigger_tau_c_over_tau_h);
  class_read_double("tight_coupling_trigger_tau_c_over_tau_k",ppr->tight_coupling_trigger_tau_c_over_tau_k);
  class_read_double("start_sources_at_tau_c_over_tau_h",ppr->start_sources_at_tau_c_over_tau_h);

  class_read_int("tight_coupling_approximation",ppr->tight_coupling_approximation);

  /** derivatives of baryon sound speed only computed if some non-minimal tight-coupling schemes is requested */
  if ((ppr->tight_coupling_approximation == (int)first_order_CLASS) || (ppr->tight_coupling_approximation == (int)second_order_CLASS)) {
    pth->compute_cb2_derivatives = _TRUE_;
  }

  class_read_int("l_max_g",ppr->l_max_g);
  class_read_int("l_max_pol_g",ppr->l_max_pol_g);
  class_read_int("l_max_ur",ppr->l_max_ur);
  if (pba->N_ncdm>0)
    class_read_int("l_max_ncdm",ppr->l_max_ncdm);
  class_read_int("l_max_g_ten",ppr->l_max_g_ten);
  class_read_int("l_max_pol_g_ten",ppr->l_max_pol_g_ten);
  class_read_double("curvature_ini",ppr->curvature_ini);
  class_read_double("entropy_ini",ppr->entropy_ini);
  class_read_double("gw_ini",ppr->gw_ini);
  class_read_double("perturb_integration_stepsize",ppr->perturb_integration_stepsize);
  class_read_double("tol_tau_approx",ppr->tol_tau_approx);
  class_read_double("tol_perturb_integration",ppr->tol_perturb_integration);
  class_read_double("perturb_sampling_stepsize",ppr->perturb_sampling_stepsize);

  class_read_int("radiation_streaming_approximation",ppr->radiation_streaming_approximation);
  class_read_double("radiation_streaming_trigger_tau_over_tau_k",ppr->radiation_streaming_trigger_tau_over_tau_k);
  class_read_double("radiation_streaming_trigger_tau_c_over_tau",ppr->radiation_streaming_trigger_tau_c_over_tau);

  class_read_int("ur_fluid_approximation",ppr->ur_fluid_approximation);
  class_read_int("ncdm_fluid_approximation",ppr->ncdm_fluid_approximation);
  class_read_double("ur_fluid_trigger_tau_over_tau_k",ppr->ur_fluid_trigger_tau_over_tau_k);
  class_read_double("ncdm_fluid_trigger_tau_over_tau_k",ppr->ncdm_fluid_trigger_tau_over_tau_k);

  class_test(ppr->ur_fluid_trigger_tau_over_tau_k==ppr->radiation_streaming_trigger_tau_over_tau_k,
             errmsg,
             "please choose different values for precision parameters ur_fluid_trigger_tau_over_tau_k and radiation_streaming_trigger_tau_over_tau_k, in order to avoid switching two approximation schemes at the same time");

  if (pba->N_ncdm>0) {

    class_test(ppr->ncdm_fluid_trigger_tau_over_tau_k==ppr->radiation_streaming_trigger_tau_over_tau_k,
               errmsg,
               "please choose different values for precision parameters ncdm_fluid_trigger_tau_over_tau_k and radiation_streaming_trigger_tau_over_tau_k, in order to avoid switching two approximation schemes at the same time");
    
    class_test(ppr->ncdm_fluid_trigger_tau_over_tau_k==ppr->ur_fluid_trigger_tau_over_tau_k,
               errmsg,
               "please choose different values for precision parameters ncdm_fluid_trigger_tau_over_tau_k and ur_fluid_trigger_tau_over_tau_k, in order to avoid switching two approximation schemes at the same time");
    
  }
  
  /** h.4. parameter related to the Bessel functions */

  class_read_double("l_logstep",ppr->l_logstep);
  class_read_int("l_linstep",ppr->l_linstep);
  class_read_double("bessel_x_step",ppr->bessel_x_step);
  class_read_double("bessel_j_cut",ppr->bessel_j_cut);
  class_read_double("bessel_tol_x_min",ppr->bessel_tol_x_min);
  class_read_string("bessel_file_name",ppr->bessel_file_name);

  /** h.5. parameter related to the primordial spectra */

  class_read_double("k_per_decade_primordial",ppr->k_per_decade_primordial);
  class_read_double("primordial_inflation_ratio_min",ppr->primordial_inflation_ratio_min);
  class_read_double("primordial_inflation_ratio_max",ppr->primordial_inflation_ratio_max);
  class_read_int("primordial_inflation_phi_ini_maxit",ppr->primordial_inflation_phi_ini_maxit);
  class_read_double("primordial_inflation_pt_stepsize",ppr->primordial_inflation_pt_stepsize);
  class_read_double("primordial_inflation_bg_stepsize",ppr->primordial_inflation_bg_stepsize);
  class_read_double("primordial_inflation_tol_integration",ppr->primordial_inflation_tol_integration);
  class_read_double("primordial_inflation_attractor_precision_pivot",ppr->primordial_inflation_attractor_precision_pivot);
  class_read_double("primordial_inflation_attractor_precision_initial",ppr->primordial_inflation_attractor_precision_initial);
  class_read_int("primordial_inflation_attractor_maxit",ppr->primordial_inflation_attractor_maxit);
  class_read_double("primordial_inflation_jump_initial",ppr->primordial_inflation_jump_initial);
  class_read_double("primordial_inflation_tol_curvature",ppr->primordial_inflation_tol_curvature);

  /** h.6. parameter related to the transfer functions */

  class_read_double("k_step_trans_scalars",ppr->k_step_trans_scalars);
  class_read_double("k_step_trans_tensors",ppr->k_step_trans_tensors);
  class_read_int("transfer_cut",ppr->transfer_cut);
  class_read_double("transfer_cut_threshold_osc",ppr->transfer_cut_threshold_osc);
  class_read_double("transfer_cut_threshold_cl",ppr->transfer_cut_threshold_cl);
  class_read_double("l_switch_limber",ppr->l_switch_limber);
  class_read_double("l_switch_limber_for_cl_density_over_z",ppr->l_switch_limber_for_cl_density_over_z);
  class_read_double("selection_cut_at_sigma",ppr->selection_cut_at_sigma);
  class_read_double("selection_sampling",ppr->selection_sampling);
  class_read_double("selection_sampling_bessel",ppr->selection_sampling_bessel);
  class_read_double("selection_tophat_edge",ppr->selection_tophat_edge);

  /** h.7. parameters related to nonlinear calculations */

  class_read_double("halofit_dz",ppr->halofit_dz);
  class_read_double("halofit_min_k_nonlinear",ppr->halofit_min_k_nonlinear);
  class_read_double("halofit_sigma_precision",ppr->halofit_sigma_precision);

  class_read_int("double escape",ppr->double_escape);
  class_read_double("z_ini",ppr->z_ini);
  class_read_int("eta_size",ppr->eta_size);
  class_read_double("k_L",ppr->k_L);
  class_read_double("k_min",ppr->k_min);
  class_read_double("logstepx_min",ppr->logstepx_min);
  class_read_double("logstepk1",ppr->logstepk1);
  class_read_double("logstepk2",ppr->logstepk2);
  class_read_double("logstepk3",ppr->logstepk3);
  class_read_double("logstepk4",ppr->logstepk4);
  class_read_double("logstepk5",ppr->logstepk5);
  class_read_double("logstepk6",ppr->logstepk6);
  class_read_double("logstepk7",ppr->logstepk7);
  class_read_double("logstepk8",ppr->logstepk8);
  class_read_double("k_growth_factor",ppr->k_growth_factor);
  class_read_double("k_scalar_max_for_pk_nl",ppr->k_scalar_max_for_pk_nl);

  if ((pnl->method==nl_trg_one_loop) ||
      (pnl->method==nl_trg)) {

    /* when using the trg module, the following parameters need to
       be changed */

    ppt->k_scalar_kmax_for_pk 
      = max(
            ppt->k_scalar_kmax_for_pk,
            ppr->k_scalar_max_for_pk_nl*pba->h);

    psp->z_max_pk = ppr->z_ini+1.;

  }
  
  /** h.8. parameter related to lensing */

  class_read_int("accurate_lensing",ppr->accurate_lensing);
  class_read_int("delta_l_max",ppr->delta_l_max);
  if (ppr->accurate_lensing == _TRUE_) {
    class_read_int("num_mu_minus_lmax",ppr->num_mu_minus_lmax);
    class_read_int("tol_gauss_legendre",ppr->tol_gauss_legendre);
  }
  if (ple->has_lensed_cls == _TRUE_)
    ppt->l_scalar_max+=ppr->delta_l_max;

  /* check various l_max */

  pbs->l_max=0;
  pbs->x_max=0;

  if (ppt->has_cls == _TRUE_) {

    if (ppt->has_scalars == _TRUE_) {
      
      if ((ppt->has_cl_cmb_temperature == _TRUE_) || 
          (ppt->has_cl_cmb_polarization == _TRUE_) || 
          (ppt->has_cl_cmb_lensing_potential == _TRUE_))
        pbs->l_max=max(ppt->l_scalar_max,pbs->l_max);

      if ((ppt->has_cl_lensing_potential == _TRUE_) || 
          (ppt->has_cl_density == _TRUE_))
        pbs->l_max=max(ppt->l_lss_max,pbs->l_max);

      pbs->x_max=max(pbs->l_max*ppr->k_scalar_max_tau0_over_l_max,pbs->x_max);
      
    }
    
    if (ppt->has_tensors == _TRUE_) {   
      pbs->l_max=max(ppt->l_tensor_max,pbs->l_max);

      pbs->x_max=max(pbs->l_max*ppr->k_tensor_max_tau0_over_l_max,pbs->x_max);
    }
  }

  pbs->x_step = ppr->bessel_x_step;

  pbs->x_max = ((int)(pbs->x_max * 1.01 / pbs->x_step)+1)*pbs->x_step;

  /** (i) shall we wrtie background quantitites in a file? */

  class_call(parser_read_string(pfc,"write background",&string1,&flag1,errmsg),
             errmsg,
             errmsg);

  if ((flag1 == _TRUE_) && ((strstr(string1,"y") != NULL) || (strstr(string1,"Y") != NULL))) {

    pop->write_background = _TRUE_;

  }

  /** (j) eventually write all the read parameters in a file */

  class_call(parser_read_string(pfc,"write parameters",&string1,&flag1,errmsg),
             errmsg,
             errmsg);	

  if ((flag1 == _TRUE_) && ((strstr(string1,"y") != NULL) || (strstr(string1,"Y") != NULL))) {

    sprintf(param_output_name,"%s%s",pop->root,"parameters.ini");
    sprintf(param_unused_name,"%s%s",pop->root,"unused_parameters");

    class_open(param_output,param_output_name,"w",errmsg);
    class_open(param_unused,param_unused_name,"w",errmsg);

    fprintf(param_output,"# List of input/precision parameters actually read\n");
    fprintf(param_output,"# (all other parameters set to default values)\n");
    fprintf(param_output,"# Obtained with CLASS %s (for developpers: svn version %s)\n",_VERSION_,_SVN_VERSION_);
    fprintf(param_output,"#\n");
    fprintf(param_output,"# This file can be used as the input file of another run\n");
    fprintf(param_output,"#\n");

    fprintf(param_unused,"# List of input/precision parameters passed\n");
    fprintf(param_unused,"# but not used (just for info)\n");
    fprintf(param_unused,"#\n");

    for (i=0; i<pfc->size; i++) {
      if (pfc->read[i] == _TRUE_)
        fprintf(param_output,"%s = %s\n",pfc->name[i],pfc->value[i]);
      else
        fprintf(param_unused,"%s = %s\n",pfc->name[i],pfc->value[i]);
    }
    fprintf(param_output,"#\n");

    fclose(param_output);
    fclose(param_unused);
  }

  return _SUCCESS_;

}

/** 
 * All default parameter values (for input parameters)
 *
 * @param pba Input : pointer to background structure 
 * @param pth Input : pointer to thermodynamics structure 
 * @param ppt Input : pointer to perturbation structure
 * @param pbs Input : pointer to bessels structure
 * @param ptr Input : pointer to transfer structure 
 * @param ppm Input : pointer to primordial structure 
 * @param psp Input : pointer to spectra structure
 * @param pop Input : pointer to output structure
 * @return the error status
 */

int input_default_params(
                         struct background *pba,
                         struct thermo *pth,
                         struct perturbs *ppt,
                         struct bessels * pbs,
                         struct transfers *ptr,
                         struct primordial *ppm,
                         struct spectra *psp,
                         struct nonlinear * pnl,
                         struct lensing *ple,
                         struct output *pop
                         ) {

  double sigma_B; /**< Stefan-Boltzmann constant in W/m^2/K^4 = Kg/K^4/s^3 */

  sigma_B = 2. * pow(_PI_,5) * pow(_k_B_,4) / 15. / pow(_h_P_,3) / pow(_c_,2);

  /** - background structure */
      
  pba->h = 0.704;
  pba->H0 = pba->h * 1.e5 / _c_;
  pba->T_cmb = 2.726;
  pba->Omega0_g = (4.*sigma_B/_c_*pow(pba->T_cmb,4.)) / (3.*_c_*_c_*1.e10*pba->h*pba->h/_Mpc_over_m_/_Mpc_over_m_/8./_PI_/_G_);
  pba->Omega0_ur = 3.04*7./8.*pow(4./11.,4./3.)*pba->Omega0_g;
  pba->Omega0_b = 0.02253/0.704/0.704;
  pba->Omega0_cdm = 0.1122/0.704/0.704;
  pba->N_ncdm = 0;
  pba->Omega0_ncdm_tot = 0.;
  pba->ksi_ncdm_default = 0.;
  pba->ksi_ncdm = NULL;
  pba->T_ncdm_default = pow(4.0/11.0,1.0/3.0);
  pba->T_ncdm = NULL;
  pba->deg_ncdm_default = 1.;
  pba->deg_ncdm = NULL;
  pba->ncdm_psd_parameters = NULL;
  pba->ncdm_psd_files = NULL;

  pba->Omega0_k = 0.;
  pba->Omega0_lambda = 1.+pba->Omega0_k-pba->Omega0_g-pba->Omega0_ur-pba->Omega0_b-pba->Omega0_cdm-pba->Omega0_ncdm_tot;
  pba->Omega0_fld = 0.;     
  pba->a_today = 1.;       
  pba->w0_fld=-1.;
  pba->wa_fld=0.;
  pba->cs2_fld=1.;

  /** - thermodynamics structure */

  pth->YHe=_BBN_;
  pth->recombination=recfast;
  pth->reio_parametrization=reio_camb;
  pth->reio_z_or_tau=reio_z;
  pth->z_reio=10.3;
  pth->tau_reio=0.085;
  pth->reionization_exponent=1.5;
  pth->reionization_width=1.5;
  pth->helium_fullreio_redshift=3.5;
  pth->helium_fullreio_width=0.5;

  pth->binned_reio_num=0;
  pth->binned_reio_z=NULL;
  pth->binned_reio_xe=NULL;
  pth->binned_reio_step_sharpness = 0.3;

  pth->annihilation = 0.;
  pth->decay = 0.;
  pth->annihilation_variation = 0.;
  pth->annihilation_z = 1000.;
  pth->annihilation_zmax = 2500.;
  pth->annihilation_zmin = 30.;
  pth->annihilation_f_halo = 0.;
  pth->annihilation_z_halo = 30.;
  pth->has_on_the_spot = _TRUE_;

  pth->compute_cb2_derivatives=_FALSE_;

  /** - perturbation structure */

  ppt->has_cl_cmb_temperature = _FALSE_;
  ppt->has_cl_cmb_polarization = _FALSE_;
  ppt->has_cl_cmb_lensing_potential = _FALSE_;
  ppt->has_cl_density = _FALSE_;
  ppt->has_cl_lensing_potential = _FALSE_;
  ppt->has_pk_matter = _FALSE_;
  ppt->has_density_transfers = _FALSE_;
  ppt->has_velocity_transfers = _FALSE_;

  ppt->has_ad=_TRUE_;  
  ppt->has_bi=_FALSE_;
  ppt->has_cdi=_FALSE_;
  ppt->has_nid=_FALSE_;
  ppt->has_niv=_FALSE_;

  ppt->has_scalars=_TRUE_;  
  ppt->has_vectors=_FALSE_;
  ppt->has_tensors=_FALSE_;  

  ppt->l_scalar_max=2500;
  ppt->l_tensor_max=500;
  ppt->l_lss_max=300;
  ppt->k_scalar_kmax_for_pk=0.1;


  ppt->alpha =2e-2 ;
  ppt->beta =1e-2;
  ppt->lambda=1e-2;
  ppt->Y_dm=1e-1;

  ppt->gauge=1;

  /** - bessels structure */

  pbs->l_max = max(max(ppt->l_scalar_max,ppt->l_tensor_max),ppt->l_lss_max);
  pbs->bessel_always_recompute = _TRUE_;

  /** - primordial structure */

  ppm->primordial_spec_type = analytic_Pk;
  ppm->k_pivot = 0.002;
  ppm->A_s = 2.42e-9;
  ppm->n_s = 0.967;
  ppm->alpha_s = 0.;
  ppm->f_bi = 1.;
  ppm->n_bi = 1.;
  ppm->alpha_bi = 0.;
  ppm->f_cdi = 1.;
  ppm->n_cdi = 1.;
  ppm->alpha_cdi = 0.;
  ppm->f_nid = 1.;
  ppm->n_nid = 1.;
  ppm->alpha_nid = 0.;
  ppm->f_niv = 1.;
  ppm->n_niv = 1.;
  ppm->alpha_niv = 0.;
  ppm->c_ad_bi = 0.;
  ppm->n_ad_bi = 0.;
  ppm->alpha_ad_bi = 0.;
  ppm->c_ad_cdi = 0.;
  ppm->n_ad_cdi = 0.;
  ppm->alpha_ad_cdi = 0.;
  ppm->c_ad_nid = 0.;
  ppm->n_ad_nid = 0.;
  ppm->alpha_ad_nid = 0.;
  ppm->c_ad_niv = 0.;
  ppm->n_ad_niv = 0.;
  ppm->alpha_ad_niv = 0.;
  ppm->c_bi_cdi = 0.;
  ppm->n_bi_cdi = 0.;
  ppm->alpha_bi_cdi = 0.;
  ppm->c_bi_nid = 0.;
  ppm->n_bi_nid = 0.;
  ppm->alpha_bi_nid = 0.;
  ppm->c_bi_niv = 0.;
  ppm->n_bi_niv = 0.;
  ppm->alpha_bi_niv = 0.;
  ppm->c_cdi_nid = 0.;
  ppm->n_cdi_nid = 0.;
  ppm->alpha_cdi_nid = 0.;
  ppm->c_cdi_niv = 0.;
  ppm->n_cdi_niv = 0.;
  ppm->alpha_cdi_niv = 0.;
  ppm->c_nid_niv = 0.;
  ppm->n_nid_niv = 0.;
  ppm->alpha_nid_niv = 0.;
  ppm->r = 1.;
  ppm->n_t = -ppm->r/8.*(2.-ppm->r/8.-ppm->n_s);
  ppm->alpha_t = ppm->r/8.*(ppm->r/8.+ppm->n_s-1.);
  ppm->potential=polynomial;
  ppm->phi_pivot=0.;
  ppm->V0=1.25e-13;
  ppm->V1=-1.12e-14;
  ppm->V2=-6.95e-14;
  ppm->V3=0.;
  ppm->V4=0.;

  /** - transfer structure */

  ppt->selection_num=1;
  ppt->selection=gaussian;
  ppt->selection_mean[0]=1.;
  ppt->selection_width[0]=0.1;
  ptr->lcmb_rescale=1.;
  ptr->lcmb_pivot=0.1;
  ptr->lcmb_tilt=0.;

  /** - output structure */ 

  pop->z_pk_num = 1;
  pop->z_pk[0] = 0.;  
  sprintf(pop->root,"output/");
  pop->write_header = _TRUE_;
  pop->output_format = class_format;
  pop->write_background = _FALSE_;

  /** - spectra structure */ 

  psp->z_max_pk = pop->z_pk[0];
  psp->non_diag=0;

  /** - nonlinear structure */

  /** - lensing structure */

  ple->has_lensed_cls = _FALSE_;
 
  /** - nonlinear structure */ 

  pnl->method = nl_none;
  pnl->ic = nl_pt;

  /** - all verbose parameters */ 

  pba->background_verbose = 0;
  pth->thermodynamics_verbose = 0;
  ppt->perturbations_verbose = 0;
  pbs->bessels_verbose = 0;
  ptr->transfer_verbose = 0;
  ppm->primordial_verbose = 0;
  psp->spectra_verbose = 0;
  pnl->nonlinear_verbose = 0;
  ple->lensing_verbose = 0;
  pop->output_verbose = 0;

  return _SUCCESS_;

}

/** 
 * Initialize the precision parameter structure. 
 * 
 * All precision parameters used in the other moduels are listed here
 * and assigned here a default value.
 *
 * @param ppr Input/Ouput: a precision_params structure pointer  
 * @return the error status
 *
 */

int input_default_precision ( struct precision * ppr ) {

  /** Summary: */

  /**
   * - parameters related to the background
   */

  ppr->a_ini_over_a_today_default = 1.e-14;
  ppr->back_integration_stepsize = 7.e-3;
  ppr->tol_background_integration = 1.e-2;

  ppr->tol_initial_Omega_r = 1.e-4;
  ppr->tol_M_ncdm = 1.e-7;
  ppr->tol_ncdm = 1.e-3;
  ppr->tol_ncdm_bg = 1.e-5;
  ppr->tol_ncdm_initial_w=1.e-3;

  /**
   * - parameters related to the thermodynamics
   */

  /* for bbn */
  sprintf(ppr->sBBN_file,"bbn/sBBN.dat");

  /* for recombination */

  ppr->recfast_z_initial=1.e4;

  ppr->recfast_Nz0=20000;
  ppr->tol_thermo_integration=1.e-2;

  ppr->recfast_Heswitch=6;                 /* from recfast 1.4 */
  ppr->recfast_fudge_He=0.86;              /* from recfast 1.4 */

  ppr->recfast_Hswitch = _TRUE_;           /* from recfast 1.5 */
  ppr->recfast_fudge_H = 1.14;             /* from recfast 1.4 */
  ppr->recfast_delta_fudge_H = -0.015;     /* from recfast 1.5.2 */
  ppr->recfast_AGauss1 = -0.14;            /* from recfast 1.5 */ 
  ppr->recfast_AGauss2 =  0.079;           /* from recfast 1.5.2 */
  ppr->recfast_zGauss1 =  7.28;            /* from recfast 1.5 */
  ppr->recfast_zGauss2 =  6.73;            /* from recfast 1.5.2 */
  ppr->recfast_wGauss1 =  0.18;            /* from recfast 1.5 */
  ppr->recfast_wGauss2 =  0.33;            /* from recfast 1.5 */

  ppr->recfast_z_He_1 = 8000.;             /* from recfast 1.4 */
  ppr->recfast_delta_z_He_1 = 50.;         /* found to be OK on 3.09.10 */
  ppr->recfast_z_He_2 = 5000.;             /* from recfast 1.4 */
  ppr->recfast_delta_z_He_2 = 100.;        /* found to be OK on 3.09.10 */
  ppr->recfast_z_He_3 = 3500.;             /* from recfast 1.4 */
  ppr->recfast_delta_z_He_3 = 50.;         /* found to be OK on 3.09.10 */
  ppr->recfast_x_He0_trigger = 0.995;      /* raised from 0.99 to 0.995 for smoother Helium */              
  ppr->recfast_x_He0_trigger2 = 0.995;     /* raised from 0.985 to same as previous one for smoother Helium */
  ppr->recfast_x_He0_trigger_delta = 0.05; /* found to be OK on 3.09.10 */
  ppr->recfast_x_H0_trigger = 0.995;       /* raised from 0.99 to 0.995 for smoother Hydrogen */
  ppr->recfast_x_H0_trigger2 = 0.995;      /* raised from 0.98 to same as previous one for smoother Hydrogen */
  ppr->recfast_x_H0_trigger_delta = 0.05;  /* found to be OK on 3.09.10 */ 

  ppr->recfast_H_frac=1.e-3;               /* from recfast 1.4 */

  sprintf(ppr->hyrec_Alpha_inf_file,"hyrec/Alpha_inf.dat");
  sprintf(ppr->hyrec_R_inf_file,"hyrec/R_inf.dat");
  sprintf(ppr->hyrec_two_photon_tables_file,"hyrec/two_photon_tables.dat");

  /* for reionization */

  ppr->reionization_z_start_max = 50.;
  ppr->reionization_sampling=1.e-2; 
  ppr->reionization_optical_depth_tol=1.e-4;
  ppr->reionization_start_factor=8.;

  /* general */

  ppr->thermo_rate_smoothing_radius=50;

  /**
   * - parameters related to the perturbations
   */

  ppr->evolver = ndf15;
  ppr->pk_definition = delta_m_squared;

  ppr->k_scalar_min_tau0=0.1;
  ppr->k_scalar_max_tau0_over_l_max=2.;
  ppr->k_scalar_step_sub=0.05;
  ppr->k_scalar_step_super=0.002;
  ppr->k_scalar_step_transition=0.2;

  ppr->k_scalar_k_per_decade_for_pk=10.;
  ppr->k_scalar_k_per_decade_for_bao=70.;
  ppr->k_scalar_bao_center=3.;
  ppr->k_scalar_bao_width=4.;

  ppr->k_tensor_min_tau0=1.4;
  ppr->k_tensor_max_tau0_over_l_max = 2.;
  ppr->k_tensor_step_sub=0.1;
  ppr->k_tensor_step_super=0.0025;
  ppr->k_tensor_step_transition=0.2;

  ppr->start_small_k_at_tau_c_over_tau_h = 0.0015;  /* decrease to start earlier in time */
  ppr->start_large_k_at_tau_h_over_tau_k = 0.07;  /* decrease to start earlier in time */
  ppr->tight_coupling_trigger_tau_c_over_tau_h=0.015; /* decrease to switch off earlier in time */
  ppr->tight_coupling_trigger_tau_c_over_tau_k=0.01; /* decrease to switch off earlier in time */
  ppr->start_sources_at_tau_c_over_tau_h = 0.008; /* decrease to start earlier in time */
  ppr->tight_coupling_approximation=(int)compromise_CLASS;

  ppr->l_max_g=10; 
  ppr->l_max_pol_g=8; 
  ppr->l_max_ur=12; 
  ppr->l_max_ncdm=12;
  ppr->l_max_g_ten=5;
  ppr->l_max_pol_g_ten=5;

  ppr->curvature_ini=1.; /* initial curvature; used to fix adiabatic initial conditions; must remain fixed to one as long as the primordial adiabatic spectrum stands for the curvature power spectrum */
  ppr->entropy_ini=1.;   /* initial entropy; used to fix isocurvature initial conditions; must remain fixed to one as long as the primordial isocurvature spectrum stands for an entropy power spectrum */
  ppr->gw_ini=0.25; /* to match normalization convention for GW in most of literature and ensure standard definition of r */

  ppr->perturb_integration_stepsize=0.5;

  ppr->tol_tau_approx=1.e-5;
  ppr->tol_perturb_integration=1.e-4;
  ppr->perturb_sampling_stepsize=0.08;

  ppr->radiation_streaming_approximation = rsa_MD_with_reio;
  ppr->radiation_streaming_trigger_tau_over_tau_k = 45.; 
  ppr->radiation_streaming_trigger_tau_c_over_tau = 5.;
 
  ppr->ur_fluid_approximation = ufa_CLASS;
  ppr->ur_fluid_trigger_tau_over_tau_k = 15.; 

  ppr->ncdm_fluid_approximation = ncdmfa_CLASS;
  ppr->ncdm_fluid_trigger_tau_over_tau_k = 16.; 

  /**
   * - parameter related to the Bessel functions
   */

  ppr->l_logstep=1.15;
  ppr->l_linstep=40;

  ppr->bessel_x_step=0.5;
  ppr->bessel_j_cut=1.e-5;
  ppr->bessel_tol_x_min =1.e-4;
  sprintf(ppr->bessel_file_name,"bessels.dat");

  /**
   * - parameter related to the primordial spectra
   */

  ppr->k_per_decade_primordial = 10.; 

  ppr->primordial_inflation_ratio_min=100.;
  ppr->primordial_inflation_ratio_max=1/50.;
  ppr->primordial_inflation_phi_ini_maxit=10000;
  ppr->primordial_inflation_pt_stepsize=0.01;
  ppr->primordial_inflation_bg_stepsize=0.005;
  ppr->primordial_inflation_tol_integration=1.e-3;
  ppr->primordial_inflation_attractor_precision_pivot=0.001;
  ppr->primordial_inflation_attractor_precision_initial=0.1;
  ppr->primordial_inflation_attractor_maxit=10000;
  ppr->primordial_inflation_jump_initial=1.2;
  ppr->primordial_inflation_tol_curvature=1.e-3;

  /**
   * - parameter related to the transfer functions
   */
  
  ppr->k_step_trans_scalars=0.4;
  ppr->k_step_trans_tensors=0.4;
  ppr->transfer_cut=tc_osc;
  ppr->transfer_cut_threshold_osc=0.007; /* 03.12.10 for chi2plT0.01 */
  ppr->transfer_cut_threshold_cl=1.e-8; /* 14.12.10 for chi2plT0.01 */

  ppr->l_switch_limber=10.;
  ppr->l_switch_limber_for_cl_density_over_z=30.;

  ppr->selection_cut_at_sigma=5.;
  ppr->selection_sampling=50;
  ppr->selection_sampling_bessel=20;
  ppr->selection_tophat_edge=0.1;

  /**
   * - parameters related to spectra module
   */

  /* nothing */

  /**
   * - parameters related to trg module
   */

  ppr->halofit_dz=0.1;
  ppr->halofit_min_k_nonlinear=0.0035;
  ppr->halofit_sigma_precision=0.05;
  ppr->double_escape=2;
  ppr->z_ini = 35.;
  ppr->eta_size = 101;
  ppr->k_L = 1.e-3;
  ppr->k_min = 1.e-4;
  ppr->logstepx_min = 1.04;
  ppr->logstepk1 = 1.11;
  ppr->logstepk2 = 0.09;
  ppr->logstepk3 = 300.;
  ppr->logstepk4 = 0.01;
  ppr->logstepk5 = 1.02;
  ppr->logstepk6 = 0.;
  ppr->logstepk7 = 0.;
  ppr->logstepk8 = 0.;
  ppr->k_growth_factor = 0.1;
  ppr->k_scalar_max_for_pk_nl = 1000.;

  /**
   * - parameter related to lensing
   */

  ppr->accurate_lensing=_FALSE_;
  ppr->num_mu_minus_lmax=70;
  ppr->delta_l_max=800;

  /**
   * - automatic estimate of machine precision
   */

  get_machine_precision(&(ppr->smallest_allowed_variation));

  class_test(ppr->smallest_allowed_variation < 0,
             ppr->error_message,
             "smallest_allowed_variation = %e < 0",ppr->smallest_allowed_variation);

  ppr->tol_gauss_legendre = ppr->smallest_allowed_variation;

  return _SUCCESS_;

}

int class_version(
                  char * version
                  ) {
  
  sprintf(version,"%s",_VERSION_);
  return _SUCCESS_;
}

/** 
 * Computes automatically the machine precision. 
 *
 * @param smallest_allowed_variation a pointer to the smallest allowed variation
 *
 * Returns the smallest
 * allowed variation (minimum epsilon * _TOLVAR_)
 */

int get_machine_precision(double * smallest_allowed_variation) {
  double one, meps, sum;
  
  one = 1.0;
  meps = 1.0;
  do {
    meps /= 2.0;
    sum = one + meps;
  } while (sum != one);
  meps *= 2.0;
  
  *smallest_allowed_variation = meps * _TOLVAR_;

  return _SUCCESS_;

}
