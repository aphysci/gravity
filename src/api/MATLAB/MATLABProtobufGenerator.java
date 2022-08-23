package com.aphysci.gravity.matlab;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.PrintStream;
import java.lang.reflect.Method;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.List;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import java.util.ArrayList;
import java.util.Arrays;

public class MATLABProtobufGenerator 
{
	public static void main(String[] args)
	{
		if (args.length !=2)
		{
			System.out.println("USAGE: MATLABProtobufGenerator [jar file] [MATLAB output src dir]");
			System.exit(0);
		}
		MATLABProtobufGenerator generator = new MATLABProtobufGenerator();
		generator.generate(args[0], args[1]);
	}
	
	private static List<String> excludeMethods = Arrays.asList("getDescriptor", "getDescriptorForType", "getDefaultInstanceForType", "getField",
															   "getRepeatedField", "getFieldBuilder", "setField", "setRepeatedField", "addRepeatedField",
															   "setUnknownFields", "getRepeatedFieldCount", "getAllFields", "getUnknownFields",
															   "getInitializationErrorString", "getClass", "clear", "clearField", "hasField",
															   "clone", "build", "buildPartial", "mergeFrom", "isInitialized", "mergeTest", 
															   "mergeUnknownFields", "findInitializationErrors", "mergeDelimitedFrom",
															   "wait", "equals", "toString", "hashCode", "notify", "notifyAll",
															   "getOneofFieldDescriptor", "clearOneof", "hasOneof");
	
	private static final String numListToArrayFcn = "com.aphysci.gravity.matlab.MATLABGravitySubscriber.convertNumberListToDoubleArray";
	private static final String numArrayToListFcn = "com.aphysci.gravity.matlab.MATLABGravitySubscriber.convertNumberArrayToNumberList";
	
	private URLClassLoader classLoader;

	public MATLABProtobufGenerator() {}

	public void generate(String jarFileName, String matlabDir)
	{
		try
		{
			// Extract list of class from specified jar file
			List<String> classNames = new ArrayList<String>();
			ZipInputStream zip = new ZipInputStream(new FileInputStream(jarFileName));
			for (ZipEntry entry = zip.getNextEntry(); entry != null; entry = zip.getNextEntry()) 
			{
				if (!entry.isDirectory() && entry.getName().endsWith(".class")) 
				{
					String className = entry.getName().replace('/', '.'); // including ".class"
					classNames.add(className.substring(0, className.length() - ".class".length()));
				}
			}
			zip.close();

			// Create ClassLoader for the specified jar
			File file = new File(jarFileName);
			URL[] urls = {file.toURI().toURL()};
			classLoader = URLClassLoader.newInstance(urls);

			// Loop over each Java class
			int classCount = 0;
			for (String className : classNames)
			{
				// We only care about Protobuf Builder classes
				if (!className.endsWith("$Builder"))
				{
					// Ignore this class
					continue;
				}
				
				// Double check and make sure the class is of the right stuff
				Class<?> c = classLoader.loadClass(className);
				if (!c.getSuperclass().getName().equals("com.google.protobuf.GeneratedMessage$Builder") && !c.getSuperclass().getName().equals("com.google.protobuf.GeneratedMessageV3$Builder"))
				{
					// Ignore
					continue;
				}				
				
				// Create the MATLAB class name from the Java class name (swapping '_'s for '$'s)
				className = className.substring(0, className.length() - 8);
				String matlabClassName = className.substring(className.indexOf("$") + 1, className.length()).replace("$", "_");
				System.out.println("Processing Java --> MATLAB (" + className + " --> " + matlabClassName + ")");
				
				// Create the writer for the new MATLAB class in the specified directory
				String matlabFileName = matlabDir + File.separator + matlabClassName + ".m";
				PrintStream writer = new PrintStream(new FileOutputStream(matlabFileName));
				
				// Create the boilerplate header stuff
				writeHeader(writer, matlabClassName, className);
				
				// Loop over each method in the Java class
				for (Method method : c.getMethods())
				{
					// Throw out listed methods that won't have equivilent MATLAB methods
					if (excludeMethods.contains(method.getName()) || method.getName().indexOf("Builder") > 0)
					{
						continue;
					}					
					if ((method.getName().startsWith("set") || method.getName().startsWith("add")) &&  
							method.getParameterTypes()[method.getParameterTypes().length - 1].getName().endsWith("$Builder"))
					{
						continue;
					}
					
					// Write the MATLAB method
					writeMethod(writer, method);

					/*
					System.out.println("\t" + Modifier.toString(method.getModifiers()) 
					+ " " + method.getName());						
					for (Class<?> param: method.getParameterTypes()) {
						String tmp = "";
						Class<?> p = param;
						if (p.getSuperclass() != null && param.getSuperclass().getName().equals("com.google.protobuf.GeneratedMessage"))
						{
							tmp += " --> " + param.getSuperclass().getName();
						}
						System.out.println("\t\t" + param.getName() + tmp);
					}
					System.out.println("\t\t == returns ==> " + method.getReturnType().getName());
					System.out.println("\t\t " + method.getGenericReturnType().getTypeName());
					*/
				}
				
				// Finish up
				writeFooter(writer);
				classCount++;
			}			
			System.out.println("Complete: Processed " + classCount + " classes");

		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}	
	
	private void writeMethod(PrintStream writer, Method method) throws Exception
	{
		String methodName = method.getName();
		int numArgs = method.getParameterTypes().length;
		String returnType = method.getReturnType().getName();

/*
if (returnType.equals("interface com.google.protobuf.ProtocolStringList"))
{
	returnType = "java.util.List<String>";
}
else if (returnType.equals("interface java.util.List"))
{
	returnType = "java.util.List<?>";
}
*/
		if (methodName.startsWith("add") && numArgs == 2)
		{
			// No overloaded methods in MATLAB
			methodName += "At";
		}
		
		// Create the method declaration
		String argsDeclaration = "";		
		for (int i = 0; i < numArgs; i++)
		{				
			argsDeclaration += ",arg" + i;
		}
		writer.println("      function ret = " + methodName + "(this" + argsDeclaration + ")");
		
		// Build up and appropriately decorate each of the arguments as used
		List<String> args = new ArrayList<String>();
		for (int i = 0; i < numArgs; i++)
		{
			String typeName = method.getGenericParameterTypes()[i].toString();
			Class<?> argClass = null;
			try
			{
				argClass = classLoader.loadClass(typeName);
			} catch (Exception e) {}
			
			String argName = "arg" + i;
			if ( ((method.getName().startsWith("set") && numArgs == 2 ) || 
			      (method.getName().startsWith("get") && numArgs == 1)) && 
				 method.getParameterTypes()[0].getName().equals("int") && i == 0)
			{
				// This is an index that needs to be modified for MATLAB to Java indexing
				args.add(argName + "-1");
			}
			else if (numArgs == 1 && method.getParameterTypes()[0].getName().equals("java.lang.Iterable"))
			{
				// We're being passed an array. Get the type of the elements
				typeName = typeName.substring(typeName.indexOf("<") + 1, typeName.indexOf(">"));
				if (typeName.contains("? extends"))
				{
					typeName = typeName.substring(10);
				}
				Class<?> typeClass = classLoader.loadClass(typeName);
				if (Integer.class.isAssignableFrom(typeClass))
				{
					// These are integers. Cast them as such.
					args.add(numArrayToListFcn + "(int32(" + argName + "))");
				}
				else if (Number.class.isAssignableFrom(typeClass))
				{
					// All other numbers will be created as doubles
					args.add(numArrayToListFcn + "(" + argName + ")");
				}
				else
				{
					// Else we're require that it be a cell array that we'll loop over and add to an ArrayList on the Java side
					writer.println("         list = java.util.ArrayList;");
					writer.println("         if iscell(arg0)");				
					writer.println("            for i = 1:length(arg0)");
					writer.println("               list.add(arg0{i});");
					writer.println("            end");
					writer.println("         else");
					writer.println("            fprintf(2, 'ERROR: argument must be cell array');");
					writer.println("         end");
					args.add("list");
				}
			}
			else if (argClass != null && argClass.getSuperclass().getName().equals("com.google.protobuf.GeneratedMessage"))
			{
				// If the argument is another protobuf message we'll extract the builder from the MATLAB class to pass to Java
				args.add(argName + ".getProtobufBuilder()");
			}
			else
			{
				// Just an ordinary argument
				args.add(argName);
			}
		}
		
		// Write the call to the underlying Java protobuf builder
		String argsString = args.toString();
		argsString = argsString.substring(1, argsString.length()-1);
		argsString = argsString.replaceAll("\\s","");
		writer.println("         ret = this.builder." + methodName + "(" + argsString + ");");
		
		// Process the return type
		Class<?> returnClass = null;
		try
		{
			returnClass = classLoader.loadClass(returnType);
		} catch (Exception e) {}		
		if (returnType.equals("java.util.List"))
		{
			// This method returns a list. Determine listed type.
			String fullTypeName = method.getGenericReturnType().toString();
	
			String listedType = null;
			Class<?> listedClass = null;
			if (fullTypeName.contains("<") && fullTypeName.contains(">"))
			{
				listedType = fullTypeName.substring(fullTypeName.indexOf("<") + 1, fullTypeName.indexOf(">"));
				listedClass = classLoader.loadClass(listedType);
			}
			if (listedClass != null && Number.class.isAssignableFrom(listedClass))
			{
				// Returns a list of numbers that we'll map to an array for MATLAB
				writer.println("         ret = " + numListToArrayFcn + "(ret);");
			}
			else if (listedClass != null && listedClass.getSuperclass().getName().equals("com.google.protobuf.GeneratedMessage"))
			{
				// Returns a list of protobufs. Loop over and create the appropriate MATLAB class for each, and add to a cell array
				String mlClass = listedType.substring(listedType.indexOf("$") + 1);
				writer.println("         list = ret;");
				writer.println("         ret = {};");
				writer.println("         for i = 1:list.size()");
				writer.println("            pb = " + mlClass + ";");
				writer.println("            pb.setProtobufBuilder(list.get(i-1));");
				writer.println("            ret{i} = pb;");
				writer.println("         end");
			}
			else
			{
				// Populate a cell array with any other content
				writer.println("         list = ret;");
				writer.println("         ret = {};");
				writer.println("         for i = 1:list.size()");
				writer.println("            ret{i} = list.get(i-1);");
				writer.println("         end");
			}
		}
		else if (returnType.equals("java.lang.String"))
		{
			// Returns a string - convert to MATLAB char array
			writer.println("      ret = char(ret);");
		}
		else if (returnClass != null && returnClass.getSuperclass() != null && returnClass.getSuperclass().getName().equals("com.google.protobuf.GeneratedMessage"))
		{
			// Returns a protobuf. Create the appropriate MATLAB class.
			String mlClass = returnType.substring(returnType.indexOf("$") + 1);
			mlClass = mlClass.replace("$", "_");
			writer.println("         pb = " + mlClass + ";");
			writer.println("         pb.setProtobufBuilder(ret);");
			writer.println("         ret = pb;");
		}
		
		// Done
		writer.println("      end");
		writer.println();
	}

	private void writeHeader(PrintStream writer, String matlabClassName, String javaClassName)
	{
		writer.println("%");
		writer.println("% Generated by the Gravity MATLABProtocolGenerator compiler.  DO NOT EDIT!");
		writer.println("%");

		writer.println("classdef " + matlabClassName + " < Protobuf");
		writer.println();

		writer.println("   properties (Access = private)");
		writer.println("      builder;");
		writer.println("   end");			    	    
		writer.println();

		writer.println("   methods (Static)");
		writer.println("      function defaultInstance = getDefaultInstance()");
		writer.println("         defaultInstance = javaMethod('getDefaultInstance', '" + javaClassName + "');");  
		writer.println("      end");
		writer.println("   end");
		writer.println();

		writer.println("   methods (Access = public)");
		writer.println("      function this = " + matlabClassName + "()");
		writer.println("         % Create Java Protobuf Builder");
		writer.println("         this.builder = javaMethod('newBuilder', '" + javaClassName + "');");

		writer.println("      end");
		writer.println();

		writer.println("      function builder = getProtobufBuilder(this)");
		writer.println("         builder = this.builder;");
		writer.println("      end");
		writer.println();

		writer.println("      function setProtobufBuilder(this, builder)");
		writer.println("         this.builder = builder;");
		writer.println("      end");
		writer.println();
	}
	
	private void writeFooter(PrintStream writer)
	{
		writer.println("   end");
		writer.println("end");
	}
}
