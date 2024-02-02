/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/* TODO: The content needs to be generated from ZAP instead of manually port */

#pragma once

#include <cstdint>
#include <string>
#include <type_traits>

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>

#include <MtClusterCommand.h>

/*----------------------------------------------------------------------------*\
| Cluster Name                                                        |   ID   |
|---------------------------------------------------------------------+--------|
| Identify                                                            | 0x0003 |
| Groups                                                              | 0x0004 |
| Scenes                                                              | 0x0005 |
| OnOff                                                               | 0x0006 |
| OnOffSwitchConfiguration                                            | 0x0007 |
| LevelControl                                                        | 0x0008 |
| BinaryInputBasic                                                    | 0x000F |
| PulseWidthModulation                                                | 0x001C |
| Descriptor                                                          | 0x001D |
| Binding                                                             | 0x001E |
| AccessControl                                                       | 0x001F |
| Actions                                                             | 0x0025 |
| BasicInformation                                                    | 0x0028 |
| OtaSoftwareUpdateProvider                                           | 0x0029 |
| OtaSoftwareUpdateRequestor                                          | 0x002A |
| LocalizationConfiguration                                           | 0x002B |
| TimeFormatLocalization                                              | 0x002C |
| UnitLocalization                                                    | 0x002D |
| PowerSourceConfiguration                                            | 0x002E |
| PowerSource                                                         | 0x002F |
| GeneralCommissioning                                                | 0x0030 |
| NetworkCommissioning                                                | 0x0031 |
| DiagnosticLogs                                                      | 0x0032 |
| GeneralDiagnostics                                                  | 0x0033 |
| SoftwareDiagnostics                                                 | 0x0034 |
| ThreadNetworkDiagnostics                                            | 0x0035 |
| WiFiNetworkDiagnostics                                              | 0x0036 |
| EthernetNetworkDiagnostics                                          | 0x0037 |
| TimeSynchronization                                                 | 0x0038 |
| BridgedDeviceBasicInformation                                       | 0x0039 |
| Switch                                                              | 0x003B |
| AdministratorCommissioning                                          | 0x003C |
| OperationalCredentials                                              | 0x003E |
| GroupKeyManagement                                                  | 0x003F |
| FixedLabel                                                          | 0x0040 |
| UserLabel                                                           | 0x0041 |
| ProxyConfiguration                                                  | 0x0042 |
| ProxyDiscovery                                                      | 0x0043 |
| ProxyValid                                                          | 0x0044 |
| BooleanState                                                        | 0x0045 |
| IcdManagement                                                       | 0x0046 |
| ModeSelect                                                          | 0x0050 |
| LaundryWasherMode                                                   | 0x0051 |
| RefrigeratorAndTemperatureControlledCabinetMode                     | 0x0052 |
| LaundryWasherControls                                               | 0x0053 |
| RvcRunMode                                                          | 0x0054 |
| RvcCleanMode                                                        | 0x0055 |
| TemperatureControl                                                  | 0x0056 |
| RefrigeratorAlarm                                                   | 0x0057 |
| DishwasherMode                                                      | 0x0059 |
| AirQuality                                                          | 0x005B |
| SmokeCoAlarm                                                        | 0x005C |
| DishwasherAlarm                                                     | 0x005D |
| OperationalState                                                    | 0x0060 |
| RvcOperationalState                                                 | 0x0061 |
| HepaFilterMonitoring                                                | 0x0071 |
| ActivatedCarbonFilterMonitoring                                     | 0x0072 |
| DoorLock                                                            | 0x0101 |
| WindowCovering                                                      | 0x0102 |
| BarrierControl                                                      | 0x0103 |
| PumpConfigurationAndControl                                         | 0x0200 |
| Thermostat                                                          | 0x0201 |
| FanControl                                                          | 0x0202 |
| ThermostatUserInterfaceConfiguration                                | 0x0204 |
| ColorControl                                                        | 0x0300 |
| BallastConfiguration                                                | 0x0301 |
| IlluminanceMeasurement                                              | 0x0400 |
| TemperatureMeasurement                                              | 0x0402 |
| PressureMeasurement                                                 | 0x0403 |
| FlowMeasurement                                                     | 0x0404 |
| RelativeHumidityMeasurement                                         | 0x0405 |
| OccupancySensing                                                    | 0x0406 |
| CarbonMonoxideConcentrationMeasurement                              | 0x040C |
| CarbonDioxideConcentrationMeasurement                               | 0x040D |
| NitrogenDioxideConcentrationMeasurement                             | 0x0413 |
| OzoneConcentrationMeasurement                                       | 0x0415 |
| Pm25ConcentrationMeasurement                                        | 0x042A |
| FormaldehydeConcentrationMeasurement                                | 0x042B |
| Pm1ConcentrationMeasurement                                         | 0x042C |
| Pm10ConcentrationMeasurement                                        | 0x042D |
| TotalVolatileOrganicCompoundsConcentrationMeasurement               | 0x042E |
| RadonConcentrationMeasurement                                       | 0x042F |
| WakeOnLan                                                           | 0x0503 |
| Channel                                                             | 0x0504 |
| TargetNavigator                                                     | 0x0505 |
| MediaPlayback                                                       | 0x0506 |
| MediaInput                                                          | 0x0507 |
| LowPower                                                            | 0x0508 |
| KeypadInput                                                         | 0x0509 |
| ContentLauncher                                                     | 0x050A |
| AudioOutput                                                         | 0x050B |
| ApplicationLauncher                                                 | 0x050C |
| ApplicationBasic                                                    | 0x050D |
| AccountLogin                                                        | 0x050E |
| ElectricalMeasurement                                               | 0x0B04 |
| UnitTesting                                                         | 0xFFF1FC05|
| FaultInjection                                                      | 0xFFF1FC06|
| SampleMei                                                           | 0xFFF1FC20|
\*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*\
| Cluster Identify                                                    | 0x0003 |
|------------------------------------------------------------------------------|
| Commands:                                                           |        |
| * Identify                                                          |   0x00 |
| * TriggerEffect                                                     |   0x40 |
|------------------------------------------------------------------------------|
| Attributes:                                                         |        |
| * IdentifyTime                                                      | 0x0000 |
| * IdentifyType                                                      | 0x0001 |
| * GeneratedCommandList                                              | 0xFFF8 |
| * AcceptedCommandList                                               | 0xFFF9 |
| * EventList                                                         | 0xFFFA |
| * AttributeList                                                     | 0xFFFB |
| * FeatureMap                                                        | 0xFFFC |
| * ClusterRevision                                                   | 0xFFFD |
|------------------------------------------------------------------------------|
| Events:                                                             |        |
\*----------------------------------------------------------------------------*/

/*
 * Command Toggle
 */
class OnOffToggle : public MtClusterCommand
{
public:
    OnOffToggle(MatterManagerCore & mtmgrCore) : MtClusterCommand(mtmgrCore)
    {
#if 0
        ClusterCommand::AddArguments();
#endif
    }

    CHIP_ERROR SendCommand(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::OnOff::Id;
        constexpr chip::CommandId commandId = chip::app::Clusters::OnOff::Commands::Toggle::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") command (0x%08" PRIX32 ") on endpoint %u", clusterId,
                        commandId, endpointIds.at(0));
        return MtClusterCommand::SendCommand(device, endpointIds.at(0), clusterId, commandId, mRequest);
    }

    CHIP_ERROR SendGroupCommand(chip::GroupId groupId, chip::FabricIndex fabricIndex) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::OnOff::Id;
        constexpr chip::CommandId commandId = chip::app::Clusters::OnOff::Commands::Toggle::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") command (0x%08" PRIX32 ") on Group %u", clusterId, commandId,
                        groupId);

        return MtClusterCommand::SendGroupCommand(groupId, fabricIndex, clusterId, commandId, mRequest);
    }

private:
    chip::app::Clusters::OnOff::Commands::Toggle::Type mRequest;
};

namespace ClusterCommands {
static CHIP_ERROR SendOnOffToggle(MatterManagerCore & mtmgrCore, chip::NodeId nodeId, chip::EndpointId endPointId)
{
    OnOffToggle cmd(mtmgrCore);
    cmd.SetDestinationId(nodeId);
    std::vector<chip::EndpointId> endPointIdList = { endPointId };
    cmd.SetEndPointId(endPointIdList);
    ReturnErrorOnFailure(cmd.Run());
    return CHIP_NO_ERROR;
}
} // namespace ClusterCommands