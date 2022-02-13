import numpy as np
from warnings import warn

#############################################
#####  Conditional stalling simulation  #####
#############################################

class Waitlist:
  def __init__(self,size,init_val=-1):
    self.EMPTY_FLAG = init_val
    self.list = [init_val for i in range(size)]
      
  def update(self,elem):
    if len(self.list)==0:
      return True
    new_val= self.EMPTY_FLAG if elem in self.list else elem
    self.list = [new_val] + self.list[0:-1]
    return new_val == elem
  
  def in_list(self,val):
    return val in self.list
  
  
def simulate_stall_stage(size,cardinality,II_update=1,seed=0,num_of_elem = 4000000, id_generator = None,fast_bubble=False):
  if id_generator is None:
    id_generator = lambda : np.random.randint(0,high=cardinality)
  np.random.seed(seed)
  
  WL= Waitlist(size,cardinality)
  cycles=0
  if fast_bubble:
    elem_id = 0
    conflict = False
    while elem_id<num_of_elem:
      if not conflict:
        new_id= id_generator()
      
      if WL.update(new_id):
        conflict = False
        elem_id +=1
        cycles +=1
        for i in range(II_update-1):
          cycles +=1
          WL.update(cardinality)
        
      else:
        conflict = True
        cycles +=1
        
  else:
    elem_id = 0
    conflict = False
    while elem_id<num_of_elem:
      if not conflict:
        new_id= id_generator()
        
      for i in range(II_update):
        if i != 0:
          WL.update(cardinality)
        else:
          if WL.update(new_id):
            conflict = False
            elem_id +=1
          else:
            conflict = True
        cycles +=1
  return cycles/num_of_elem


def time_series_simulation_stall_stage(size,cardinality,seed=0,
                                       window_size = 1000,num_of_elem = 4000000, id_generator = None):
  
  II_update=1 # only tested for II_update=1
  
  if id_generator is None:
    id_generator = lambda : np.random.randint(0,high=cardinality)
  np.random.seed(seed)
  
  WL= Waitlist(size,cardinality)
  cycles_per_elem = []
  cycles_for_window = []
  prev_cycles= None # it will fail if it's used before init
  cycles=0
  
  for elem_id in range(num_of_elem):
    elem_cycles = 0 
    for i in range(II_update):
      if i != 0:
        WL.update(cardinality)
        cycles +=1
        elem_cycles +=1
      else:
        new_id= id_generator()
        while(True):
          cycles +=1
          elem_cycles +=1
          if WL.update(new_id):
            break
    
    cycles_per_elem += [elem_cycles] # add number of cycles to process last element
    if len(cycles_per_elem) == window_size:
      cycles_for_window += [sum(cycles_per_elem)]
      cycles_per_elem = cycles_per_elem[1:]
    elif len(cycles_per_elem) >= window_size:
      assert False
  
  return cycles/num_of_elem, cycles_for_window
  
  
def get_II_CDF_from_cycle_series(samples,window_size=1000):
  samples = sorted(samples)
  delta = 1/window_size
  x_axis = np.arange(window_size,samples[-1]+1)/window_size ## II values
  samples = np.array(samples)/window_size ## has to be after the previous line
  
  CDF=[]
  acc = 0
  x_idx = 0
  sample_size = len(samples)
  for s in samples:
    while s > x_axis[x_idx]:
      CDF+=[acc/sample_size]
      x_idx +=1
    acc +=1
  CDF+=[acc/sample_size] # store last point
  
  assert acc/sample_size == 1
  x_axis = x_axis[:len(CDF)] # no need to use the whole x_axis
  
  return x_axis,CDF
  

def get_II_PDF_from_cycle_series(samples,window_size=1000):
  samples = sorted(samples)
  delta = 1/window_size
  x_axis = np.arange(window_size,samples[-1]+1)/window_size ## II values
  samples = np.array(samples)/window_size ## has to be after the previous line
  
  PDF=[]
  acc = 0
  x_idx = 0
  sample_size = len(samples)
  for s in samples:
    while s > x_axis[x_idx]:
      PDF+=[acc/sample_size]
      x_idx +=1
      acc = 0
    acc +=1
  PDF+=[acc/sample_size] # store last point
  assert abs(sum(PDF)-1) <1e-7,"sum(PDF): %f" %(sum(PDF))
  
  x_axis = x_axis[:len(PDF)] # no need to use the whole x_axis
  
  return x_axis,PDF


### Zipf probs

def rand_zipf(s,N):
  x = np.random.random()
  acc = 0
  D = sum([1/i**s for i in range(1,N+1)])
  for k in range(1,N+1):
    acc += (1/k**s)/D
    if x < acc:
      return k-1 # the kth number -> k-1 address
    
  assert False, "Error: x(%f)> acc(%f)" % (x,acc)
    
def zipf_CDF(s,N):
  CDF = []
  D = sum([1/i**s for i in range(1,N+1)])
  acc = 0
  for k in range(1,N+1):
    acc += (1/k**s)/D
    CDF += [acc]
    
  assert abs(CDF[-1]-1)<1e-5, "CDF is not good approx. CDF =  %f" % CDF
  
  return CDF


def rand_CDF(CDF):
  x = np.random.random()
  for i in range(len(CDF)):
    if x < CDF[i]:
      return i 
    
  assert False, "Error: x(%f)> higher value(%f)" % (x,CDF[-1])
  

def zipf_prob(k,s,N):
  assert 0<k<=N
  return (1/k**s)/sum([1/i**s for i in range(1,N+1)])


################################################################################ 
#####   COnditional stalling mathematical model for Uniform distribution   #####
################################################################################
def pretty_matrix_print(M):
  for row in range(len(M)):
    print(row,"|" , ["%6.4f," % M[row][col] for col in range(len(M[row])) ])
    
def get_Markov_steady_state_probs_iter(P):
  
  def check_steady_matrix(M,extrict = True,tol = 1e-6):
    for row in M:
      elem = row[0]
      for other_elem in row[1:]:
        if abs(elem-other_elem) > tol:
          if extrict:
            assert False, "The stocastic matrix needs more iterations to get the steady state. Diff: %f" % (elem-other_elem)
          else:  
            return False
        
    return True
  
  P = np.array(P)
  n=10*int(np.log2(len(P))+1) # 10 times the number of memories
  P_steady = P
  for i in range(n):
    P_steady = np.matmul(P_steady,P)
    
  #check it is steady
  Extra_iters = 50
  if not check_steady_matrix(P_steady,extrict=False):
    warn("""The stocastic matrix needs more iterations to get the steady state. 
                    Trying %d more iterations""" %(Extra_iters))
    for i in range(Extra_iters):
      P_steady = np.matmul(P_steady,P)
      
  check_steady_matrix(P_steady,extrict=True)
  steady_probs = P_steady[:,0:1] # get column
  
  return steady_probs


def get_Markov_steady_state_probs(M):
  eigenvec = None
  vals, vectors = np.linalg.eig(M)
  for i,val in enumerate(vals):
    if np.isreal(val) and np.isclose(val, 1,atol=1e-10):
      if np.isreal(vectors[:,i]).all():
        eigenvec = vectors[:,i:i+1]
        eigenvec = eigenvec.real
        eigenvec = eigenvec/sum(eigenvec)
  
  assert np.isclose(sum(eigenvec), 1,atol=1e-10)
  return eigenvec

def num_of_1s(num):
  assert num >= 0, "function only for Natural numbers"
  if num == 0:
    return 0
  bits = int(np.floor(np.log2(num)))+1
  cnt = 0 
  for i in range(bits):
    cnt += (num>>i)&1
  return cnt

def get_uniform_stocastic_matrix(L,cardinality,Pc=None):
  ''' L = latency = Number of update logic stages -1 
      cardinality = cardinality of the id source
      
      This version creates the stocastic matrix iterating
      over rows instead of columns as exaplained in the
      paper. Iterating over columns results in a probably 
      simpler code, but this code came first
  '''
  
  def get_mask_n_pattern(num,mask_size):
    bits = int(np.floor(np.log2(num)))
    colission_idx = mask_size-bits -2
    
    #mask
    mask = (2**bits-1) << (mask_size-bits)
    if colission_idx >=0:
      mask |=1<< colission_idx
      
    #pattern
    pattern = (num &(2**bits-1))<<mask_size-bits
    if colission_idx >=0:
      pattern |=1<< colission_idx
    
    return mask,pattern

  if Pc == None:
      Pc = 1/cardinality # id ~ U(1/cardinality)
  size = 2**(L-1)
  
  if size == 1:
    #single state case
    return [[1]]
  
  P = []
  P += [[Pc if i < 2**(L-2) else 2*Pc for i in range(size)]] # get first row
  for row in range(1,size//2):  
    mask,pattern = get_mask_n_pattern(row,mask_size=L-1)
    P += [[Pc if (col&mask)==pattern else 0 for col in range(size)]]
    
  mask = 2**(L-2)-1 # 011..1
  for row in range(size//2,size):
    P += [[1-Pc*(1+num_of_1s(col)) if ((col>>1)&mask)==(row&mask) else 0 for col in range(size)]]
    
  # Connect states that cannot happen dirrectly to the empty pipeline state
  for col in range(size):
    if (num_of_1s(col)+1) > cardinality:
      for row in range(size):
        P[row][col] = 1 if row == 0 else 0
  
  # check that all cols sum 1
  for col in range(size):
    prob = sum([ P[row][col] for row in range(size) ])
    if not(abs(prob-1) < 1e-7):
      pretty_matrix_print(P)
    assert abs(prob-1) < 1e-7, "Stocastic matrix is not correct: sum of col is %f" %(prob)
    
  # check that all values are between 0 and 1
  for row in range(size):
    for col in range(size):
      if not(0<= P[row][col] <=1):
        pretty_matrix_print(P)
      assert 0<= P[row][col] <=1, "Stocastic matrix is not correct: it has values outside de [0,1] range"
    
  return P

def get_emission_probs_matrix(L,C,Pc=None):
  ''' L = latency = Number of update logic stages -1 '''
  
  assert L > 0
  assert C >= 1
  
  if Pc is None:
    Pc = 1/C
  states = 2**(L-1)
  outputs = L+1
  
  #first row (no collisions)
  E = [[ 1-(num_of_1s(s)+1)*Pc if (num_of_1s(s)+1)<C else 0 for s in range(states)]]
  
  #rest (collisions)
  E += [[Pc if ((s+2**(L-1)) & 1<<(o-2))!= 0 and (num_of_1s(s)+1)<=C else 0 for s in range(states)] 
                                                      for o in range(2,outputs+1) ]
  
  #check matrix
  for s in range(states):
    col_sum = sum([E[o][s] for o in range(outputs)])
    if (num_of_1s(s)+1)<=C:
      assert abs(col_sum - 1)< 1e-9, "Error (L=%d,Pc=%6.4f): col %d sum is not 1 (sum= %6.4f)" %(L,Pc,s,col_sum)
    else:
      assert abs(col_sum )< 1e-9, "Error (L=%d,Pc=%6.4f): col %d sum is not 0 (sum= %6.4f)" %(L,Pc,s,col_sum)
  return E

def get_II_from_HMM(L,cardinality,Pc=None):
  ''' 
  Function to get the average II for and architecture with an update latency of 
  L and an id ~U(1/cardinaility)
  
      L = latency = Number of update logic stages -1 
      cardinality = cardinality of the id source
  '''
  
  assert L >= 0
  assert cardinality >= 1
  
  if L == 0:
    return 1
  
  P = get_uniform_stocastic_matrix(L,cardinality,Pc)
  E = get_emission_probs_matrix(L,cardinality,Pc)
  steady_probs = get_Markov_steady_state_probs(P)
  outputs = [x for x in range(1,L+2)]
  
  single_elem_dist = np.matmul(E,steady_probs)
  model_II = np.matmul(outputs,single_elem_dist)
  return model_II[0]



####### Conditional stalling II_sys approximation given a P_c

def approx_II(L,Pc):
  ''' get exact value or approximation of IIsys for II_p=1'''
  first_approx = lambda L,Pc :1 + (L*L + L)*Pc/2
  II_lim = 1.35
  L_lim = -.5 + .5*np.sqrt(8*(II_lim-1)/Pc+1)
  if L <= L_lim:
    return first_approx(L,Pc)
  else:
    b = (2*L_lim+1)*Pc/2
    return II_lim + b*(L-L_lim )


####### Conditional stalling II_sys approximation given a P_c
def get_eq_II(IIp,probs=None,L=None,II_eq_sys=None,fast_bubble=False):
  ''' Function to approximate the II_sys of systems with II_p !=1 using
  the II_sys computed from an somewhat equivalent system with:
  a dependency distantance DD' = floor(DD/II_p) and II_p =1. 
  If the system consumes bubbles with an II=1, then fast_bubble=True.
  For fast_bubble = False, the returned value is exact.
  For fast_bubble = True, the returned value might be exact or approx. 
  depending of the relationship between II_p and L
  
  Args:
  - II_eq_sys = II_sys' = F(DD',II_p=1, distribution )
  - IIp of the system
  - If fast_bubble=True it requires:
      * L: Latency between operations that create the dependency.
        In the case of a RAW dep: Read+ process + write latencies
      * probs: Distribution of instantaneous II, that is:
        probs[x-1]= P(II=x) for a given iteration.
        This can be obtained for the uniform distribution with the 
        HMM as probs = EM x Steady_state_dist:
          P = get_uniform_stocastic_matrix(L,cardinality,Pc)
          E = get_emission_probs_matrix(L,cardinality,Pc)
          steady_probs = get_Markov_steady_state_probs(P)
          probs = np.matmul(E,steady_probs)[:,0]
      '''
  
  if fast_bubble:
    return II_eq_sys*IIp - ((-L)%IIp)*sum([p for p in probs[1:] ])
  else:
    return II_eq_sys*IIp