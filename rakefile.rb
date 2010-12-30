############################################################################
## 
## Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). 
## All rights reserved. 
## Contact: Nokia Corporation (testabilitydriver@nokia.com) 
## 
## This file is part of Testability Driver Qt Agent
## 
## If you have questions regarding the use of this file, please contact 
## Nokia at testabilitydriver@nokia.com . 
## 
## This library is free software; you can redistribute it and/or 
## modify it under the terms of the GNU Lesser General Public 
## License version 2.1 as published by the Free Software Foundation 
## and appearing in the file LICENSE.LGPL included in the packaging 
## of this file. 
## 
############################################################################


desc "Task for cruise control"
task :cruise => ['build_visualizer'] do
	exit(0)
end

desc "Task for building the example QT application(s)"
task :build_visualizer do

  puts "#########################################################"
  puts "### Building visualizer                               ####"
  puts "#########################################################"

  make = "make"
  sudo = ""	

  if /win/ =~ RUBY_PLATFORM
    make = "mingw32-make"
  else
    sudo = "sudo -S "
  end
  cmd = sudo + " #{make} uninstall"
  system(cmd)

  cmd = "#{make} distclean"
  system(cmd)
  
  cmd = "qmake CONFIG+=release"
  failure = system(cmd)
  raise "qmake failed" if (failure != true) or ($? != 0) 
    
  cmd = "#{make}"
  failure = system(cmd)
  raise "make release failed" if (failure != true) or ($? != 0) 
    
  cmd = sudo + "#{make} install"
  failure = system(cmd)
  raise "make install failed" if (failure != true) or ($? != 0) 
  puts "visualizer built"
end




