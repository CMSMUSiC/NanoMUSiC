#include "TriggerSelector.hh"

#include <iostream>

#include "Pxl/Pxl/interface/pxl/core.hh"
#include "Pxl/Pxl/interface/pxl/hep.hh"

using std::string;

TriggerSelector::TriggerSelector( Tools::MConfig const &cfg ) :
   m_runOnData(     cfg.GetItem< bool   >( "General.RunOnData" ) ),
   m_ignoreL1(      cfg.GetItem< bool   >( "Trigger.IgnoreL1" ) ),
   m_ignoreHLT(     cfg.GetItem< bool   >( "Trigger.IgnoreHL" ) ),
   m_triggerPrefix( cfg.GetItem< string >( "Trigger.Prefix" ) + "_" ),   // This will be "HLT" in general.

   m_triggerGroups( initTriggerGroups( cfg ) )
{
}


TriggerSelector::TriggerGroupCollection TriggerSelector::initTriggerGroups( Tools::MConfig const &cfg ) const {
   TriggerGroupCollection triggerGroups;
   if( not m_ignoreHLT ) {
      // Which trigger groups shall be considered?
      std::vector< string > const groupNames = Tools::splitString< string >( cfg.GetItem< string >( "Trigger.Groups" ), true );


      std::vector< string >::const_iterator groupName;
      for( groupName = groupNames.begin(); groupName != groupNames.end(); ++groupName ) {
         // TriggerGroup knows what to do with the config.
         TriggerGroup one_group( cfg, m_triggerPrefix, *groupName );
         triggerGroups.push_back( one_group );
      }
   }

   return triggerGroups;
}

bool TriggerSelector::passHLTrigger( const std::map< std::string, std::vector< pxl::Particle* > >& particleMap,
                                     pxl::EventView *evtView
                                     ) const {
   return passHLTriggerWithoutOffline( particleMap, evtView ) and checkOfflineCut( particleMap, evtView );
}

bool TriggerSelector::passHLTriggerWithoutOffline( const std::map< std::string, std::vector< pxl::Particle* > >& particleMap,
                                                   pxl::EventView *evtView
                                                   ) const {
   // Do we care about HLTs?
   if( m_ignoreHLT ) return true;

   // Accept all events if no triggers are configured.
   if( m_triggerGroups.size() == 0 ) return true;

   // Store if any trigger of any group fired.
   bool accept = false;

   // For each trigger group, check if any of the triggers has fired and if the event
   // topology fulfills the trigger requirements.
   for( TriggerGroupCollection::const_iterator group = m_triggerGroups.begin(); group != m_triggerGroups.end(); ++group ) {
      TriggerGroup const &this_group = *group;

      bool const group_required = this_group.getRequire();

      TriggerGroup::TriggerResults const triggerResults = this_group.getTriggerResults( evtView );

      bool const any_trigger_fired = anyTriggerFired( triggerResults );

      if( any_trigger_fired ) accept = true;
      if( group_required and not any_trigger_fired )
      {
         accept = false;
         break;
      }
   }

   // If any required group didn't fire, do not accept the event.
   // If no group is required, any fired trigger is fine.
   return accept;
}

bool TriggerSelector::checkOfflineCut( const std::map< std::string, std::vector< pxl::Particle* > >& particleMap,
                                       pxl::EventView *evtView ) const
{
   bool accept = false;
   for( TriggerGroupCollection::const_iterator group = m_triggerGroups.begin(); group != m_triggerGroups.end(); ++group )
   {
      bool passed = false;
      bool const trigger_particle_accepted = group->passTriggerParticles( particleMap );
      auto const triggerResults = group->getTriggerResults( evtView );
      bool const group_required = group->getRequire();

      if ( trigger_particle_accepted and anyTriggerFired( triggerResults ) )
      {
         accept = true;
         passed = true;
      }
      if ( group_required and not passed )
         return false;
   }
   return accept;
}

bool TriggerSelector::passEventTopology( const std::map< std::string, int >& count_map ) const {
   bool any_accepted = false;

   // Check topology for all given trigger groups.
   for( TriggerGroupCollection::const_iterator group = m_triggerGroups.begin(); group != m_triggerGroups.end(); ++group ) {
      if( (*group).checkTopology( count_map ) ) any_accepted = true;
      // No need to proceed once we found one.
      if( any_accepted ) break;
   }

   return any_accepted;
}


bool TriggerSelector::checkVeto( const std::map< std::string, std::vector< pxl::Particle* > >& particleMap,
                                 pxl::EventView const *evtView
                                 ) const {
   // If triggers ignored, no veto.
   if( m_ignoreHLT ) return false;

   // Veto no events if no triggers are configured.
   if( m_triggerGroups.size() == 0 ) return false;

   // Store if any trigger group causes a veto.
   bool any_veto = false;

   // Loop over groups...
   for( TriggerGroupCollection::const_iterator group = m_triggerGroups.begin(); group != m_triggerGroups.end(); ++group ) {
      // If we found one, no need to proceed.
      if( any_veto ) break;

      TriggerGroup const &this_group = *group;

      bool const trigger_reject = this_group.getReject();

      if( not trigger_reject ) continue;  // Nothing to do here...

      TriggerGroup::TriggerResults const triggerResults = this_group.getTriggerResults( evtView );
      // Did any trigger in this group fire?
      bool const any_trigger_fired = anyTriggerFired( triggerResults );

      // Do not call passTriggerParticles if none of the triggers in this trigger group fired anyway.
      bool const trigger_particle_accepted = any_trigger_fired ? this_group.passTriggerParticles( particleMap ) : false;

      // Reject the event if the trigger fired.
      if( trigger_reject and any_trigger_fired and trigger_particle_accepted ) any_veto = true;
   }

   return any_veto;
}

bool TriggerSelector::anyTriggerFired( TriggerGroup::TriggerResults const &triggerResults ) const {
   bool any_trigger_fired = false;

   TriggerGroup::TriggerResults::const_iterator triggerResult;
   for( triggerResult = triggerResults.begin(); triggerResult != triggerResults.end(); ++triggerResult ) {
      if( (*triggerResult).second == 1 ) any_trigger_fired = true;
      // No need to continue once we found one.
      if( any_trigger_fired ) break;
   }

   return any_trigger_fired;
}
