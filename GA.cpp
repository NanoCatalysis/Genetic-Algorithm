/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/
# Implementation of the Genetic Algorithm (GA) algorithm for structure optimization
#
# Version X.X Octuber 2020
#
# BH algorithm has been implemented using C++;
# coupled to FHI-aims X.X (DFT code as calculator)
# It also works with newest XX version, XX
#
# Author : Jorge Refugio Fabila Fabian <jorge_fabila@ciencias.unam.mx> (IF-UNAM)
# Advisor : Dr. Oliver Paz Borbon <oliver_paz@fisica.unam.mx> (IF-UNAM)
#
# Note: Output folders will be generated in current directory
\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
#include"atomic.hpp"
string Simbolo_1, Simbolo_2, file_name, command, aux,geometry_file;
string initialization_file, outputfile, m_str, i_str, E_str, tag;
int continue_alg,  Ncore, randomness, kick, iteraciones,swap_step, contenido, previus;
int m, lj, N_Simbolo_1, N_Simbolo_2, count, fail_counter=0, resto, failed_max,crystal;
int n_pop, element;
float step_width, Temperature, Energy, Energia, EnergiaAnterior, k_BT, damp ;
float x_min,y_min,z_min,x_max,y_max,z_max;
Cluster clus_1, clus_2, c_aux;
Crystal cristal;
float dist;
int main(int argc, char *argv[]){
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//                                    Gets data from input.bh                                     //
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
Simbolo_1=string_pipe("grep 'cluster_ntyp' input.bh | cut -d '[' -f 2 | cut -d ':' -f 1 ");
Simbolo_2=string_pipe("grep 'cluster_ntyp' input.bh | cut -d '[' -f 3 | cut -d ':' -f 1 ");
N_Simbolo_1=int_pipe("grep 'cluster_ntyp' input.bh | cut -d '[' -f 2 | cut -d ':' -f 2 | cut -d ']' -f 1 ");
N_Simbolo_2=int_pipe("grep 'cluster_ntyp' input.bh | cut -d '[' -f 3 | cut -d ':' -f 2 | cut -d ']' -f 1 ");
continue_alg=int_pipe("grep 'continue' input.bh | cut -d \"=\" -f2 | awk '{print $1}' ");
n_pop=int_pipe("grep 'n_pop' input.bh | cut -d \"=\" -f2 | awk '{print $1}' ");
fit_function=int_pipe("grep 'fit_function' input.bh | cut -d \"=\" -f2 | awk '{print $1}' ");
initialization_file=string_pipe("grep 'initialization_file' input.bh | cut -d \"=\" -f2 | awk '{print $1}'");
randomness=int_pipe("grep 'randomness' input.bh | cut -d \"=\" -f2 | awk '{print $1}'");
kick=int_pipe("grep 'kick_type' input.bh  | cut -d \"=\" -f2 | awk '{print $1}' ");
file_name=string_pipe("grep 'directory_name' input.bh | cut -d \"=\" -f2 | awk '{print $1}' ");
step_width=float_pipe("grep 'step_width' input.bh | cut -d \"=\" -f2 | awk '{print $1}' ");
Temperature=float_pipe("grep 'temperature_K' input.bh | cut -d \"=\" -f2 | awk '{print $1}' ");
Ncore=int_pipe("grep 'Ncore' input.bh | head -1 | cut -d \"=\" -f2 | awk '{print $1}' ");
iteraciones=int_pipe("grep 'iterations' input.bh | cut -d \"=\" -f2 | awk '{print $1}' ");
swap_step=int_pipe("grep 'swap_step' input.bh | cut -d \"=\" -f2 | awk '{print $1}' ");
lj=int_pipe("grep 'lennard-jones_aid' input.bh | cut -d \"=\" -f2 | awk '{print $1}' ");
crystal=int_pipe("cd input ; if [ -f crystal.in ]  ; then echo 1  ;  fi ");
Cluster clus[n_pop];
double Fit[n_pop], rho[n_pop], a_exp, a_lin, fit_max, fit_min;
double Energies[n_pop], seleccion[n_pop], max_tmp, min_tmp, current;
// Meta-parámetros /////
failed_max=3;         //
damp=0.7;             //
dist=1.0;             //
a_exp=-3.0;           //
a_lin=0.7;            //
////////////////////////
srand(time(NULL)); // init  Randomness
//Automatically detects if exists a crystal file
if(crystal==1)  //Esto sustituye tener que poner [x_min,x_max]; [y_min,y_max]... en el input
{
  cout<<" --> Reading crystal from file "<<endl;
  cristal.read_fhi("input/crystal.in");
  x_min=cristal.x_min();
  x_max=cristal.x_max();
  y_min=cristal.y_min();
  y_max=cristal.y_max();
  z_min=cristal.z_min();
  z_max=cristal.z_max();
}
else
{
  cout<<" --> crystal.in file not found ... performing gas phase search "<<endl;
}
int i = 1;

if(continue_alg==1)
{
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  //                                      RESTART ALGORITHM                                         //
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  //En vez de reinicio podriamos hacer que directamente lo genera desde aca
  cout<<" --> Restarting algorithm ...  "<<endl;
      string iteration_counter_i ="cd ";
             iteration_counter_i+=file_name;
             iteration_counter_i+=" ; ls Generation* | wc -l";
  i=int_pipe(iteration_counter_i,1);
      string iteration_counter_m ="cd ";
             iteration_counter_m+=file_name;
             iteration_counter_m+="/Generation"+to_string(i)+" ; ls -R | grep \"relaxed.xyz\" | wc -l"; //el -R nos da libertad de escoger nombres
  m=int_pipe(iteration_counter_m,0);
     command.clear(); command=" cd "+file_name+"/Generation"+to_string(i)+" ; head -2 current_minimum.xyz | tail -1 | awk '{print $6 }' ";
     //RECUERDA: en cada generacion poner un current_minimum
     Energy=float_pipe(command);
     command.clear();

  //cout<<" --> Last Generation i="<<i<<" ; last rejected m="<<m<<" ; total performed steps : "<<i+m<<endl;
  cout<<" --> Last Generation i="<<i<<" ; "<<m<<"/"<<n_pop<<" relaxations performed : "<<i<<endl;
  cout<<" --> Restarting from generation"<<i<<" and relaxation "<<m<<" "<<endl;
//  i++;
//  cout<<" --> Starting step "<<i<<endl;
  m++;
}
else
{
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  //                                        BEGIN ALGORITHM                                         //
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
   cout<<" --> Starting a new search "<<endl;
   // Creates work directory
   command ="if [ -d "+file_name+" ] ; then mv "+file_name+" other_"+file_name;
   command+=" ; fi ; mkdir "+file_name+" ; cd "+file_name+"  ; mkdir Generation1 ;";
   command+=" cp ../input/* . ;";
   system(command.c_str());
   i=1; m=0;
   contenido=0;
//   while(contenido!=1)
//   {
      if(initialization_file.length() > 5)
      {
        cout<<" --> Reading initialization file from:  "<<initialization_file<<endl;
        //Generates geometry.in and then run FHI-aims, if geometry.in.next_step is not
        //genereted then anyway here is created as a copy of the original.
        /////////////////////////////////
        command.clear(); command="cd "+file_name+"/Generation1 ; mkdir E"+to_string(m);
        command+="cp ";                //
        command+=initialization_file;  //
        command+=" "+file_name+"/Generation1/E1/geometry.in";
        system(command.c_str());       //
        command.clear();               //
        /////////////////////////////////
        command ="cp ";                //
        command+=initialization_file;  //
        command+=" "+file_name+"/Generation1/E1/geometry.in.next_step";
        system(command.c_str());       //
        command.clear();               //
        /////////////////////////////////
        cout<<" --> Generating a random population and adding initialization file from "<<initialization_file<<endl;
        count=1;
      }
      else
      {
//Genera Generation1/E1/geometry.in aleatorio
         cout<<" --> Generating a random population "<<endl;
         count=0;
      }
      for(element=count;element<n_pop;element++)
      {
         if(N_Simbolo_2>0)  // For bimetallic cases
         {
            if(randomness==1)  // Fully random
            {
               cout<<"   --> Using fully random generator "<<endl;
               clus[element].srand_generator(Simbolo_1,N_Simbolo_1,Simbolo_2,N_Simbolo_2);
               if(lj!=0)
               {
                  cout<<"   --> Optimizing geometry with L-J potential "<<endl;
                  clus[element].geometry_optimization();
               }
            }
            else if(randomness==0)//pseudorandomly (cuts Au80 cluster)
            {
               cout<<"   --> Cleaving Au80 cluster until get a "<<Simbolo_1<<N_Simbolo_1<<Simbolo_2<<N_Simbolo_2<<" new cluster "<<endl;
               clus[element].rand_generator(Simbolo_1,N_Simbolo_1,Simbolo_2,N_Simbolo_2);
               if(lj!=0)
               {
                  cout<<"   --> Optimizing geometry with L-J potential "<<endl;
                  clus[element].geometry_optimization();
               }
            }
            else if(randomness==2)// Roy-based generator
            {
               cout<<"   --> Using random generator based on Roy Jhonston "<<endl;
               clus[element].roy_generator(Simbolo_1,N_Simbolo_1,Simbolo_2,N_Simbolo_2);
               if(lj!=0)
               {
                  cout<<"   --> Optimizing geometry with L-J potential "<<endl;
                  clus[element].geometry_optimization();
               }
            }
         }
         else //Monometallic cases
         {
            if(randomness==1)  // fully random
            {
               cout<<"   --> Using fully random generator "<<endl;
               clus[element].srand_generator(Simbolo_1,N_Simbolo_1);
               if(lj!=0)
               {
                  cout<<"   --> Optimizing geometry with L-J potential "<<endl;
                  clus[element].geometry_optimization();
               }
            }
            else if(randomness==0)//pseudorandomly (cuts Au80 cluster)
            {
               cout<<"   --> Cleaving Au80 cluster until get a "<<Simbolo_1<<N_Simbolo_1<<" new cluster "<<endl;
               clus[element].rand_generator(Simbolo_1,N_Simbolo_1);
               if(lj!=0)
               {
                  cout<<"   --> Optimizing geometry with L-J potential "<<endl;
                  clus[element].geometry_optimization();
               }
            }
            else if(randomness==2)// Roy-based generator
            {
               cout<<"   --> Using random generator based on Roy Jhonston "<<endl;
               clus[element].roy_generator(Simbolo_1,N_Simbolo_1,Simbolo_2,N_Simbolo_2);
               if(lj!=0)
               {
                  cout<<"   --> Optimizing geometry with L-J potential "<<endl;
                  clus[element].geometry_optimization();
               }
            }
         }
         command.clear(); command="cd "+file_name+"/Generation1 ; mkdir E"+to_string(element);
         system(command.c_str());
         command.clear();
         if(crystal==0)
         {
            clus[element].centroid();
            geometry_file.clear();
            geometry_file=file_name+"/Generation1/E"+to_string(element)+"/geometry.in";
            clus[element].print_fhi(geometry_file);
         }
         else{
            clus[element].centroid();
            clus[element].move((x_max-x_min)/2.0+random_number(-dist,dist),(y_max-y_min)/2.0+random_number(-dist,dist),z_max-clus[element].z_min());
            geometry_file.clear();
            geometry_file=file_name+"/Generation1/E"+to_string(element)+"/geometry.in";
            clus[element].print_fhi(geometry_file);
            command="cat "+file_name+"/crystal.in > "+geometry_file+" ; sed '/atom/a initial_moment 0.5' "+file_name;
            command+="/geometry.tmp >> "+geometry_file+" ; rm "+file_name+"/geometry.tmp" ;
            system(command.c_str());
            command.clear();
         }
      }
      m=0;
//// ACA TERMINA EL FOR que genera la generacion inicial
}
      cout<<" --> Starting FHI-aims calculations "<<endl;

      command="echo 'Step ----> Energy[eV]' >> "+file_name+"/Generation1/energies.txt ";
      system(command.c_str());
      command.clear();
cout<<"Entering loop"<<endl;
// running and colecting energies
for(m=m;m<n_pop;m++)
{
  cout<<m<<endl;
      command.clear();
      command="cd "+file_name+"/Generation1/E"+to_string(m)+" ; cp ../../run.sh .";
      command+=" ; cp ../../control.in .";
      system(command.c_str());
      command.clear();
      command="cd "+file_name+"/Generation1/E"+to_string(m)+" ; ./run.sh";
      system(command.c_str());
      command.clear();
      // Tal vez contenido ya no se use
      command="grep 'Have a nice day' "+file_name+"/Generation1/E"+to_string(m)+"/output.out | wc -l";
      contenido=int_pipe(command.c_str());
      command.clear();
   /// Store energy and optimized geometry:
   command="grep \" | Total energy of the DFT \" "+file_name+"/Generation1/E"+to_string(m)+"/output.out | awk '{print $12}' ";
/////////////////////////////////////////
/////////////////////////////////////////
/////////////////////////////////////////
// Esto es lo k esta dando lata porque no existe el output.out con la E
   Energies[m]=double_pipe(command.c_str());
   m_str=to_string(m);
   E_str=string_pipe(command); //Better for Energies with all the value
   command.clear();
   // get initial, relaxed geometry and store it as xyz (first xyz!)
   command=file_name+"/Generation1/E"+to_string(m)+"/geometry.in.next_step";
   clus[element].read_fhi(command);
   command.clear();
   command=file_name+"/Generation1/E"+to_string(m)+"/relaxed_coordinates.xyz";
   tag.clear();
   tag=" Iteration "+m_str+" -----> Energy = "+E_str+" eV ";
   clus[element].print_xyz(command,tag);
   command.clear();

   command="echo '"+to_string(m)+" ---->' "+E_str+" >> "+file_name+"/Generation1/energies.txt";
   system(command.c_str());
   command.clear();
}
// Sorts energies:
// Get Maximum
   max_tmp=Energies[0];
   for(j=1;j<n_pop;j++)
   {
      current=Energies[j];
      if( current > max_tmp )
      {
         max_tmp=current;
      }
   }
// Get Minimum
   min_tmp=Energies[0];
   for(j=1;j<n_pop;j++)
   {
      current=Energies[j];
      if( current > min_tmp )
      {
         min_tmp=current;
      }
   }
   //Calcula rho
   for(j=1;j<n_pop;j++)
   {
      rho[j]=(Energies[j]-min_tmp)/(max_tmp-min_tmp);
   }
   // rho_max=1; rho_min=0;
   //Calcula fit
   for(j=1;j<n_pop;j++)
   {
      if(fit_function==0) // Exponential
      {
         Fit[j]=exp(a_exp*rho[j]);
         fit_max=
      }
      else if(fit_function==1) // Linear
      {
         Fit[j]=1-(a_lin*rho[j]);
      }
      else if(fit_function==2) // tanh
      {
         Fit[j]=(0.5)*(1-tanh(2.0*rho[j]-1));
      }
   }
   //Establece los parámetros, rangos y porcentajes
      if(fit_function==0) // Exponential
      {
         fit_max=exp(a_exp*1.0);
         fit_min=exp(a_exp*0.0);
      }
      else if(fit_function==1) // Linear
      {
         fit_max=1-(a_lin*1.0);
         fit_min=1-(a_lin*0.0);
      }
      else if(fit_function==2) // tanh
      {
         fit_max=(0.5)*(1-tanh(2.0-1));
         fit_min=(0.5)*(1-tanh(-1.0));
      }
   //Seleccion de los especímenes
   for(j=1;j<n_pop;j++)
   {
      if(Fit[j]>(fit_min+(fit_max-fit_min)*0.8))  // 20% sobrevive tal cual
      {
         seleccion[j]="sobrevive";
      }
      else if((Fit[j]>(fit_min+(fit_max-fit_min)*0.5)) && (Fit[j]<(fit_min+(fit_max-fit_min)*0.8))) // %30 pasa sus hijos
      {
         seleccion[j]="crossover";
      }
      else if(Fit[j]<=(fit_min+(fit_max-fit_min)*0.5) // %50 mutan: 20% kick; 10% swap; 20% twist??
      {
         if(N_Simbolo_2>0) // Si es bimetálico se utiliza el swap
         {
            if(random_number(0,1)<0.75)
            {
               seleccion[j]="kick";
            }
            else
            {
               seleccion[j]="swap";
            }
         }
         else
         {
            seleccion[j]="kick";
         }
   }

   // Starting main GA loop

   cout<<" --> Initial generation: DONE! "<<endl;
   cout<<" --> AG-DFT routine starts here "<<endl;
   cout<<"  "<<endl;
   cout<<" --> Starting generation 2 "<<endl;

   cout<<"================================================================================================"<<endl;
   cout<<"GA-DFT routine starts here! "<<endl;
   /*cout<<"Note: "<<endl;
   cout<<"For monometallic clusters: only random xyz moves will be applied "<<endl;
   cout<<"For bimetallic clusters  : 1 atomic swap will be performed after "<<swap_step<<" moves "<<endl;
   cout<<"================================================================================================"<<endl;*/
   i=2;

   while( i<iteraciones)
   {
      // Gets energies from last Generation and calculates fits
Energies
      // Sort the configurations

      // Generate  new Generation
      // Run new generation
      /*
      if(criteria==0)
      {
         break;
      }
      */
   }
/*
while(i+m <= iteraciones)
{
  /// Instead of kick / swap write a new loop (over elements of generation)
  /// In order to get energies, fitting and mates
  resto=i%swap_step;

  i_str.clear();
  E_str.clear();
  i_str=to_string(i);

  geometry_file.clear();
  geometry_file=file_name+"/coordinates"+to_string(i-1)+".xyz";
  c_aux.read_xyz(geometry_file);
  geometry_file.clear();
  geometry_file=file_name+"/aux.fhi";
  c_aux.print_fhi(geometry_file);

  // For bimetallics:
  if(N_Simbolo_2>0)
  {
   clus_1=extract(geometry_file,Simbolo_1);
   clus_2=extract(geometry_file,Simbolo_2);

   clus  =clus_1+clus_2;
  }
  // for monometallics:
  else
  {
   clus=extract(geometry_file,Simbolo_1);
  }
  if(resto==0 && N_Simbolo_2 > 0)
  {
    cout<<"   --> Atoms swapped "<<endl;
    clus.type = "bimetallic";

    if(N_Simbolo_1>=N_Simbolo_2)
    {
       clus.swap(N_Simbolo_2);
    }
    else
    {
       clus.swap(N_Simbolo_1);
    }
  }
  else
  {
    if(kick==0)
    {
      // random kick:
      cout<<"   --> Kicking configuration with step width "<<step_width<<endl;
      clus.kick(step_width);
    }
    else
    {
      // damped kick (jorge):
      cout<<"   --> Kicking configuration with L-J potential aid "<<endl;
      clus.simulated_annealing(step_width);
    }
  }
  // Update geometries if in the gas-phase:
  if(crystal==0)
   {
      clus.centroid();
      geometry_file.clear();
      geometry_file=file_name+"/geometry.in";
      clus.print_fhi(geometry_file);
   }

  //.. or periodic case:
  else
   {
      clus.centroid();
      clus.move((x_max-x_min)/2.0+random_number(-dist,dist),(y_max-y_min)/2.0+random_number(-dist,dist),z_max-clus.z_min() );
      geometry_file.clear();
      geometry_file=file_name+"/geometry.tmp";
      clus.print_fhi(geometry_file);
      command="cat "+file_name+"/crystal.in > "+file_name+"/geometry.in ; sed '/atom/a initial_moment 0.5' "+file_name;
      command+="/geometry.tmp >> "+file_name+"/geometry.in ; rm "+file_name+"/geometry.tmp" ;
      system(command.c_str());
      command.clear();
   }

  ////////////////////////////////////////////
  cout<<"   --> Starting FHI-aims calculation  "<<endl;
  command.clear();
  command="cd "+file_name+" ; ./run.sh";
  system(command.c_str());
  command.clear();
  command="grep 'Have a nice day' "+file_name+"/output.out | wc -l";
  contenido=int_pipe(command.c_str());
  command.clear();
  // If structure DID NOT converged, then:
  while (contenido!=1)
  {
     cout<<"     --> SCF failed. A new configuration will be created  "<<endl;
     cout<<"         Note: while failing the SCF calculation the step width will increase"<<endl;
     if(fail_counter<failed_max)
     {
        fail_counter++;
        geometry_file.clear();
        geometry_file=file_name+"/coordinates"+to_string(i-1)+".xyz";
        c_aux.read_xyz(geometry_file);
        geometry_file.clear();
        geometry_file=file_name+"/aux.fhi";
        c_aux.print_fhi(geometry_file);

     //  Bimetallic
        if(N_Simbolo_2>0)
        {
           clus_1=extract(geometry_file,Simbolo_1);
           clus_2=extract(geometry_file,Simbolo_2);
           clus  =clus_1+clus_2;
        }
        // for monometallics:
        else
        {
           clus=extract(geometry_file,Simbolo_1);
        }
        command.clear();
        command="rm "+geometry_file;
        system(command.c_str());
        // Applies swap or kick
        if(resto==0 && N_Simbolo_2 > 0)
        {
           cout<<"     --> Applying a new swap step"<<endl;
           clus.type = "bimetallic";
           if(N_Simbolo_1>=N_Simbolo_2)
           {
              clus.swap(N_Simbolo_1);
           }
           else
           {
              clus.swap(N_Simbolo_2);
           }
        }
        else
        {
           if(kick==0)
           { // increase kick with damp to explore PES
              cout<<"     --> Applying a new kick step with step width "<<fail_counter*damp<<endl;
              clus.kick(step_width+(fail_counter*damp));
           }
           else
           { // increase kick with damp to explore PES
              cout<<"     --> Applying a new kick step with step width "<<fail_counter*damp<<" and J-L aid "<<endl;
              clus.simulated_annealing(fail_counter*damp);
           }
        }
     }
     else
     {
        cout<<"     --> All the attempts failed ... "<<endl;
        cout<<"     --> Starting again from randomly generated structure"<<endl;
        if(N_Simbolo_2>0)
        {
           if(randomness==1)  // Fully random
           {
              cout<<"   --> Using fully random generator "<<endl;
              clus.srand_generator(Simbolo_1,N_Simbolo_1,Simbolo_2,N_Simbolo_2);
              if(lj!=0)
              {
                 cout<<"   --> Optimizing geometry with L-J potential "<<endl;
                 clus.geometry_optimization();
              }
           }
           else if(randomness==0)//pseudorandomly (cuts Au80 cluster)
           {
              cout<<"   --> Cleaving Au80 cluster until get a "<<Simbolo_1<<N_Simbolo_1<<Simbolo_2<<N_Simbolo_2<<" new cluster "<<endl;
              clus.rand_generator(Simbolo_1,N_Simbolo_1,Simbolo_2,N_Simbolo_2);
              if(lj!=0)
              {
                 cout<<"   --> Optimizing geometry with L-J potential "<<endl;
                 clus.geometry_optimization();
              }
           }
           else if(randomness==2)// Roy-based generator
           {
              cout<<"   --> Using random generator based on Roy Jhonston "<<endl;
              clus.roy_generator(Simbolo_1,N_Simbolo_1,Simbolo_2,N_Simbolo_2);
              if(lj!=0)
              {
                 cout<<"   --> Optimizing geometry with L-J potential "<<endl;
                 clus.geometry_optimization();
              }
           }
        }
        else //Monometallic cases
        {
           if(randomness==1)  // fully random
           {
              cout<<"   --> Using fully random generator "<<endl;
              clus.srand_generator(Simbolo_1,N_Simbolo_1);
              if(lj!=0)
              {
                 cout<<"   --> Optimizing geometry with L-J potential "<<endl;
                 clus.geometry_optimization();
              }
           }
           else if(randomness==0)//pseudorandomly (cuts Au80 cluster)
           {
              cout<<"   --> Cleaving Au80 cluster until get a "<<Simbolo_1<<N_Simbolo_1<<" new cluster "<<endl;
              clus.rand_generator(Simbolo_1,N_Simbolo_1);
              if(lj!=0)
              {
                 cout<<"   --> Optimizing geometry with L-J potential "<<endl;
                 clus.geometry_optimization();
              }
           }
           else if(randomness==2)// Roy-based generator
           {
              cout<<"   --> Using random generator based on Roy Jhonston "<<endl;
              clus.roy_generator(Simbolo_1,N_Simbolo_1,Simbolo_2,N_Simbolo_2);
              if(lj!=0)
              {
                 cout<<"   --> Optimizing geometry with L-J potential "<<endl;
                 clus.geometry_optimization();
              }
           }
        }
     }

     if(crystal==0)
     {
        clus.centroid();
        geometry_file.clear();
        geometry_file=file_name+"/geometry.in";
        clus.print_fhi(geometry_file);
     }
     else
     {
        clus.centroid();
        clus.move((x_max-x_min)/2.0+random_number(-dist,dist),(y_max-y_min)/2.0+random_number(-dist,dist),z_max-clus.z_min() );
        geometry_file.clear();
        geometry_file=file_name+"/geometry.tmp";
        clus.print_fhi(geometry_file);
        command="cat "+file_name+"/crystal.in > "+file_name+"/geometry.in ; sed '/atom/a initial_moment 0.5' "+file_name;
        command+="/geometry.tmp >> "+file_name+"/geometry.in ; rm "+file_name+"/geometry.tmp" ;
        system(command.c_str());
        command.clear();
     }
     cout<<"   --> Starting FHI-aims calculation  "<<endl;
     command.clear();
     command="cd "+file_name+" ; ./run.sh";
     system(command.c_str());
     command.clear();
     command="grep 'Have a nice day' "+file_name+"/output.out | wc -l";
     contenido=int_pipe(command.c_str());
     command.clear();
   }

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//                                         SAVE ENERGIES                                          //
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

EnergiaAnterior=Energy;
command="grep \" | Total energy of the DFT \" "+file_name+"/output.out | awk '{print $12}' ";
Energy=float_pipe(command.c_str(),EnergiaAnterior);
E_str=string_pipe(command,"Energy couldn't be obtained");
command.clear();

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//                                     Metropolis Monte-Carlo                                     //
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
k_BT = 0.00008617 * Temperature;
if (pow(2.71,(EnergiaAnterior-Energy)/k_BT) > random_number(0,1))
{
  cout<<"   --> Basin Hopping MC criteria: Energy accepted! "<<endl;
  cout<<"   --> Saving energies ..."<<endl;
  command=file_name+"/geometry.in.next_step";
  clus.read_fhi(command);
  command.clear();

  command=file_name+"/coordinates"+i_str+".xyz";
  tag.clear();
  tag=" Iteration "+i_str+" -----> Energy = "+E_str+" eV ";
  clus.print_xyz(command,tag);
  command.clear();

  command="mv "+file_name+"/output.out "+file_name+"/output"+i_str+".out";
  system(command.c_str());
  command.clear();

  command="mv "+file_name+"/geometry.in "+file_name+"/geometry"+i_str+".in";
  system(command.c_str());
  command.clear();

  command="echo "+i_str+" '---->'  "+E_str+" >> "+file_name+"/energies.txt";
  system(command.c_str());
  command.clear();
  command="tail -"+i_str+"  "+file_name+"/energies.txt |  sort -nk3 > "+file_name+"/sorted.txt";
  system(command.c_str());
  command.clear();
  cout<<"   --> Finished iteration "<<i<<endl;
  if(i+m+1 <= iteraciones)
  {
     cout<<" --> Starting step "<<i+1<<endl;
  }
  i++;
  fail_counter=0;
}
else
{
  cout<<"   --> Basin Hopping MC criteria: Energy rejected!"<<endl;
  m++;
  string m_str = to_string(m);
  command=file_name+"/geometry.in.next_step";
  clus.read_fhi(command);
  command.clear();
  command ="if [ -f "+file_name+"/rejected/geometry_rejected"+i_str;
  command+=".in  ]; then ls "+file_name+"/rejected/geometry_rejected"+i_str+"*.in | wc -l ; else echo 0 ; fi";
  previus=int_pipe(command);
  command.clear();
  if(previus>0)
  {

     command=file_name+"/rejected/coordinate_rejected"+i_str+"("+to_string(previus)+").xyz";
     tag.clear();
     tag=" Iteration rejected "+i_str+"("+to_string(previus)+") -----> Energy = "+E_str+" eV ";
     clus.print_xyz(command,tag);
     command.clear();

     command="mv "+file_name+"/output.out '"+file_name+"/rejected/output_rejected"+i_str+"("+to_string(previus)+").out'";
     system(command.c_str());
     command.clear();

     command="mv "+file_name+"/geometry.in '"+file_name+"/rejected/geometry_rejected"+i_str+"("+to_string(previus)+").in'";
     system(command.c_str());
     command.clear();
     m_str.clear();
  }
  else
  {
     command=file_name+"/rejected/coordinate_rejected"+i_str+".xyz";
     tag.clear();
     tag=" Iteration rejected "+i_str+" -----> Energy = "+E_str+" eV ";
     clus.print_xyz(command,tag);
     command.clear();

     command="mv "+file_name+"/output.out "+file_name+"/rejected/output_rejected"+i_str+".out";
     system(command.c_str());
     command.clear();

     command="mv "+file_name+"/geometry.in "+file_name+"/rejected/geometry_rejected"+i_str+".in";
     system(command.c_str());
     command.clear();
     m_str.clear();
  }
  fail_counter=0;
}

} // END OF BH-LOOP
cout<<" --> Maximum steps reached ... Stopping Basin Hopping algorithm"<<endl;
*/
return 0;
}
