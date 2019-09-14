/*********************************************
 * OPL 12.7.1.0 Model
 * Author: dengqi
 * Creation Date: 2019年9月14日 at 下午9:07:39
 *********************************************/
main{
	var source = new IloOplModelSource("problem3.mod"); 
 	var def = new IloOplModelDefinition(source);
 	var ofile = new IloOplOutputFile("./phy_load_x.txt");
 	var dataFileName = "problem3.dat";
 	var data = new IloOplDataSource(dataFileName);
	var opl = new IloOplModel(def,cplex);
	opl.addDataSource(data);
	opl.generate();
	if (cplex.solve()) 
		{
			//print phy
			for(var v=0;v<opl.S;v++){
				ofile.write(opl.phy[v]);
				ofile.write(" ");
			}					
			ofile.write("\n");	
			//print load
			for(var v=0;v<opl.S;v++){
				ofile.write(opl.load[v]);
				ofile.write(" ");
			}					
			ofile.write("\n");	
			//print x
			for(var i=0;i<opl.F;i++)	{
				for(var v=0;v<opl.S;v++){
					ofile.write(opl.x[i][v]);
					ofile.write(" ");
   				}					
				ofile.write("\n");
    		}						
		}
		else
		{
			ofile.write("error");		
		}
}