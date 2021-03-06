addpath("../../MATLAB/dcsimsep/");
ps=case2383_mod_ps();
blackouts=load("BOpairs.mat");
bopairs=blackouts.BOpairs;

[n m]=size(bopairs);
bo_buses=zeros(n,4);

bo_buses(:,1)=ps.branch(bopairs(:,1),1);
bo_buses(:,2)=ps.branch(bopairs(:,1),2);
bo_buses(:,3)=ps.branch(bopairs(:,2),1);
bo_buses(:,4)=ps.branch(bopairs(:,2),2);

bo_out=fopen('bopairs.txt','w');
fprintf(bo_out,"%d\n",n);
for i=1:n
	fprintf(bo_out,"%d %d %d %d\n",bo_buses(i,:));
end
