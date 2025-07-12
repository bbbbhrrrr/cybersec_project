#include <stdio.h>
#include <iostream>
#include <assert.h>
#include "circom.hpp"
#include "calcwit.hpp"
void Poseidon2Constants_0_create(uint soffset,uint coffset,Circom_CalcWit* ctx,std::string componentName,uint componentFather);
void Poseidon2Constants_0_run(uint ctx_index,Circom_CalcWit* ctx);
void PowerFive_1_create(uint soffset,uint coffset,Circom_CalcWit* ctx,std::string componentName,uint componentFather);
void PowerFive_1_run(uint ctx_index,Circom_CalcWit* ctx);
void Poseidon2Round_2_create(uint soffset,uint coffset,Circom_CalcWit* ctx,std::string componentName,uint componentFather);
void Poseidon2Round_2_run(uint ctx_index,Circom_CalcWit* ctx);
void Poseidon2Round_3_create(uint soffset,uint coffset,Circom_CalcWit* ctx,std::string componentName,uint componentFather);
void Poseidon2Round_3_run(uint ctx_index,Circom_CalcWit* ctx);
void Poseidon2Permutation_4_create(uint soffset,uint coffset,Circom_CalcWit* ctx,std::string componentName,uint componentFather);
void Poseidon2Permutation_4_run(uint ctx_index,Circom_CalcWit* ctx);
void Poseidon2Hash_5_create(uint soffset,uint coffset,Circom_CalcWit* ctx,std::string componentName,uint componentFather);
void Poseidon2Hash_5_run(uint ctx_index,Circom_CalcWit* ctx);
void Poseidon2ZK_6_create(uint soffset,uint coffset,Circom_CalcWit* ctx,std::string componentName,uint componentFather);
void Poseidon2ZK_6_run(uint ctx_index,Circom_CalcWit* ctx);
Circom_TemplateFunction _functionTable[7] = { 
Poseidon2Constants_0_run,
PowerFive_1_run,
Poseidon2Round_2_run,
Poseidon2Round_3_run,
Poseidon2Permutation_4_run,
Poseidon2Hash_5_run,
Poseidon2ZK_6_run };
Circom_TemplateFunction _functionTableParallel[7] = { 
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL };
uint get_main_input_signal_start() {return 1;}

uint get_main_input_signal_no() {return 3;}

uint get_total_signal_no() {return 1680;}

uint get_number_of_components() {return 148;}

uint get_size_of_input_hashmap() {return 256;}

uint get_size_of_witness() {return 627;}

uint get_size_of_constants() {return 18;}

uint get_size_of_io_map() {return 2;}

uint get_size_of_bus_field_map() {return 0;}

void release_memory_component(Circom_CalcWit* ctx, uint pos) {{

if (pos != 0){{

if(ctx->componentMemory[pos].subcomponents)
delete []ctx->componentMemory[pos].subcomponents;

if(ctx->componentMemory[pos].subcomponentsParallel)
delete []ctx->componentMemory[pos].subcomponentsParallel;

if(ctx->componentMemory[pos].outputIsSet)
delete []ctx->componentMemory[pos].outputIsSet;

if(ctx->componentMemory[pos].mutexes)
delete []ctx->componentMemory[pos].mutexes;

if(ctx->componentMemory[pos].cvs)
delete []ctx->componentMemory[pos].cvs;

if(ctx->componentMemory[pos].sbct)
delete []ctx->componentMemory[pos].sbct;

}}


}}


// function declarations
// template declarations
void Poseidon2Constants_0_create(uint soffset,uint coffset,Circom_CalcWit* ctx,std::string componentName,uint componentFather){
ctx->componentMemory[coffset].templateId = 0;
ctx->componentMemory[coffset].templateName = "Poseidon2Constants";
ctx->componentMemory[coffset].signalStart = soffset;
ctx->componentMemory[coffset].inputCounter = 0;
ctx->componentMemory[coffset].componentName = componentName;
ctx->componentMemory[coffset].idFather = componentFather;
ctx->componentMemory[coffset].subcomponents = new uint[0];
Poseidon2Constants_0_run(coffset,ctx);
}

void Poseidon2Constants_0_run(uint ctx_index,Circom_CalcWit* ctx){
FrElement* circuitConstants = ctx->circuitConstants;
FrElement* signalValues = ctx->signalValues;
FrElement expaux[7];
FrElement lvar[4];
u64 mySignalStart = ctx->componentMemory[ctx_index].signalStart;
std::string myTemplateName = ctx->componentMemory[ctx_index].templateName;
std::string myComponentName = ctx->componentMemory[ctx_index].componentName;
u64 myFather = ctx->componentMemory[ctx_index].idFather;
u64 myId = ctx_index;
u32* mySubcomponents = ctx->componentMemory[ctx_index].subcomponents;
bool* mySubcomponentsParallel = ctx->componentMemory[ctx_index].subcomponentsParallel;
std::string* listOfTemplateMessages = ctx->listOfTemplateMessages;
uint sub_component_aux;
uint index_multiple_eq;
int cmp_index_ref_load = -1;
{
PFrElement aux_dest = &lvar[0];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[0]);
}
{
PFrElement aux_dest = &lvar[1];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[1]);
}
{
PFrElement aux_dest = &signalValues[mySignalStart + 0];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[3]);
}
{
PFrElement aux_dest = &signalValues[mySignalStart + 1];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[5]);
}
{
PFrElement aux_dest = &signalValues[mySignalStart + 2];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[7]);
}
{
PFrElement aux_dest = &signalValues[mySignalStart + 3];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[8]);
}
{
PFrElement aux_dest = &signalValues[mySignalStart + 4];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[9]);
}
{
PFrElement aux_dest = &signalValues[mySignalStart + 5];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[10]);
}
{
PFrElement aux_dest = &lvar[2];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[6]);
}
Fr_lt(&expaux[0],&lvar[2],&circuitConstants[1]); // line circom 30
while(Fr_isTrue(&expaux[0])){
{
PFrElement aux_dest = &lvar[3];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[2]);
}
Fr_lt(&expaux[0],&lvar[3],&circuitConstants[0]); // line circom 31
while(Fr_isTrue(&expaux[0])){
{
PFrElement aux_dest = &signalValues[mySignalStart + (((3 * Fr_toInt(&lvar[2])) + (1 * Fr_toInt(&lvar[3]))) + 0)];
// load src
Fr_mul(&expaux[5],&lvar[2],&circuitConstants[12]); // line circom 32
Fr_add(&expaux[3],&circuitConstants[11],&expaux[5]); // line circom 32
Fr_mul(&expaux[4],&lvar[3],&circuitConstants[13]); // line circom 32
Fr_add(&expaux[2],&expaux[3],&expaux[4]); // line circom 32
Fr_add(&expaux[1],&expaux[2],&lvar[2]); // line circom 32
Fr_add(&expaux[0],&expaux[1],&lvar[3]); // line circom 32
// end load src
Fr_copy(aux_dest,&expaux[0]);
}
{
PFrElement aux_dest = &lvar[3];
// load src
Fr_add(&expaux[0],&lvar[3],&circuitConstants[4]); // line circom 31
// end load src
Fr_copy(aux_dest,&expaux[0]);
}
Fr_lt(&expaux[0],&lvar[3],&circuitConstants[0]); // line circom 31
}
{
PFrElement aux_dest = &lvar[2];
// load src
Fr_add(&expaux[0],&lvar[2],&circuitConstants[4]); // line circom 30
// end load src
Fr_copy(aux_dest,&expaux[0]);
}
Fr_lt(&expaux[0],&lvar[2],&circuitConstants[1]); // line circom 30
}
for (uint i = 0; i < 0; i++){
uint index_subc = ctx->componentMemory[ctx_index].subcomponents[i];
if (index_subc != 0)release_memory_component(ctx,index_subc);
}
}

void PowerFive_1_create(uint soffset,uint coffset,Circom_CalcWit* ctx,std::string componentName,uint componentFather){
ctx->componentMemory[coffset].templateId = 1;
ctx->componentMemory[coffset].templateName = "PowerFive";
ctx->componentMemory[coffset].signalStart = soffset;
ctx->componentMemory[coffset].inputCounter = 1;
ctx->componentMemory[coffset].componentName = componentName;
ctx->componentMemory[coffset].idFather = componentFather;
ctx->componentMemory[coffset].subcomponents = new uint[0];
}

void PowerFive_1_run(uint ctx_index,Circom_CalcWit* ctx){
FrElement* circuitConstants = ctx->circuitConstants;
FrElement* signalValues = ctx->signalValues;
FrElement expaux[1];
FrElement lvar[0];
u64 mySignalStart = ctx->componentMemory[ctx_index].signalStart;
std::string myTemplateName = ctx->componentMemory[ctx_index].templateName;
std::string myComponentName = ctx->componentMemory[ctx_index].componentName;
u64 myFather = ctx->componentMemory[ctx_index].idFather;
u64 myId = ctx_index;
u32* mySubcomponents = ctx->componentMemory[ctx_index].subcomponents;
bool* mySubcomponentsParallel = ctx->componentMemory[ctx_index].subcomponentsParallel;
std::string* listOfTemplateMessages = ctx->listOfTemplateMessages;
uint sub_component_aux;
uint index_multiple_eq;
int cmp_index_ref_load = -1;
{
PFrElement aux_dest = &signalValues[mySignalStart + 2];
// load src
Fr_mul(&expaux[0],&signalValues[mySignalStart + 1],&signalValues[mySignalStart + 1]); // line circom 59
// end load src
Fr_copy(aux_dest,&expaux[0]);
}
{
PFrElement aux_dest = &signalValues[mySignalStart + 3];
// load src
Fr_mul(&expaux[0],&signalValues[mySignalStart + 2],&signalValues[mySignalStart + 2]); // line circom 60
// end load src
Fr_copy(aux_dest,&expaux[0]);
}
{
PFrElement aux_dest = &signalValues[mySignalStart + 0];
// load src
Fr_mul(&expaux[0],&signalValues[mySignalStart + 3],&signalValues[mySignalStart + 1]); // line circom 61
// end load src
Fr_copy(aux_dest,&expaux[0]);
}
for (uint i = 0; i < 0; i++){
uint index_subc = ctx->componentMemory[ctx_index].subcomponents[i];
if (index_subc != 0)release_memory_component(ctx,index_subc);
}
}

void Poseidon2Round_2_create(uint soffset,uint coffset,Circom_CalcWit* ctx,std::string componentName,uint componentFather){
ctx->componentMemory[coffset].templateId = 2;
ctx->componentMemory[coffset].templateName = "Poseidon2Round";
ctx->componentMemory[coffset].signalStart = soffset;
ctx->componentMemory[coffset].inputCounter = 6;
ctx->componentMemory[coffset].componentName = componentName;
ctx->componentMemory[coffset].idFather = componentFather;
ctx->componentMemory[coffset].subcomponents = new uint[3]{0};
}

void Poseidon2Round_2_run(uint ctx_index,Circom_CalcWit* ctx){
FrElement* circuitConstants = ctx->circuitConstants;
FrElement* signalValues = ctx->signalValues;
FrElement expaux[4];
FrElement lvar[4];
u64 mySignalStart = ctx->componentMemory[ctx_index].signalStart;
std::string myTemplateName = ctx->componentMemory[ctx_index].templateName;
std::string myComponentName = ctx->componentMemory[ctx_index].componentName;
u64 myFather = ctx->componentMemory[ctx_index].idFather;
u64 myId = ctx_index;
u32* mySubcomponents = ctx->componentMemory[ctx_index].subcomponents;
bool* mySubcomponentsParallel = ctx->componentMemory[ctx_index].subcomponentsParallel;
std::string* listOfTemplateMessages = ctx->listOfTemplateMessages;
uint sub_component_aux;
uint index_multiple_eq;
int cmp_index_ref_load = -1;
{
PFrElement aux_dest = &lvar[0];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[4]);
}
{
PFrElement aux_dest = &lvar[1];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[0]);
}
{
uint aux_create = 0;
int aux_cmp_num = 0+ctx_index+1;
uint csoffset = mySignalStart+15;
uint aux_dimensions[1] = {3};
for (uint i = 0; i < 3; i++) {
std::string new_cmp_name = "PowerFive_29_806"+ctx->generate_position_array(aux_dimensions, 1, i);
PowerFive_1_create(csoffset,aux_cmp_num,ctx,new_cmp_name,myId);
mySubcomponents[aux_create+ i] = aux_cmp_num;
csoffset += 4 ;
aux_cmp_num += 1;
}
}
{
PFrElement aux_dest = &lvar[2];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[2]);
}
{
PFrElement aux_dest = &lvar[3];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[2]);
}
Fr_lt(&expaux[0],&lvar[3],&circuitConstants[0]); // line circom 19
while(Fr_isTrue(&expaux[0])){
{
PFrElement aux_dest = &signalValues[mySignalStart + ((1 * Fr_toInt(&lvar[3])) + 9)];
// load src
Fr_add(&expaux[0],&signalValues[mySignalStart + ((1 * Fr_toInt(&lvar[3])) + 3)],&signalValues[mySignalStart + ((1 * Fr_toInt(&lvar[3])) + 6)]); // line circom 20
// end load src
Fr_copy(aux_dest,&expaux[0]);
}
{
PFrElement aux_dest = &lvar[3];
// load src
Fr_add(&expaux[0],&lvar[3],&circuitConstants[4]); // line circom 19
// end load src
Fr_copy(aux_dest,&expaux[0]);
}
Fr_lt(&expaux[0],&lvar[3],&circuitConstants[0]); // line circom 19
}
{
PFrElement aux_dest = &lvar[3];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[2]);
}
Fr_lt(&expaux[0],&lvar[3],&circuitConstants[0]); // line circom 28
while(Fr_isTrue(&expaux[0])){
{
uint cmp_index_ref = ((1 * Fr_toInt(&lvar[2])) + 0);
{
PFrElement aux_dest = &ctx->signalValues[ctx->componentMemory[mySubcomponents[cmp_index_ref]].signalStart + 1];
// load src
// end load src
Fr_copy(aux_dest,&signalValues[mySignalStart + ((1 * Fr_toInt(&lvar[3])) + 9)]);
}
// run sub component if needed
if(!(ctx->componentMemory[mySubcomponents[cmp_index_ref]].inputCounter -= 1)){
PowerFive_1_run(mySubcomponents[cmp_index_ref],ctx);

}
}
{
PFrElement aux_dest = &signalValues[mySignalStart + ((1 * Fr_toInt(&lvar[3])) + 12)];
// load src
cmp_index_ref_load = ((1 * Fr_toInt(&lvar[2])) + 0);
cmp_index_ref_load = ((1 * Fr_toInt(&lvar[2])) + 0);
// end load src
Fr_copy(aux_dest,&ctx->signalValues[ctx->componentMemory[mySubcomponents[((1 * Fr_toInt(&lvar[2])) + 0)]].signalStart + 0]);
}
{
PFrElement aux_dest = &lvar[3];
// load src
Fr_add(&expaux[0],&lvar[3],&circuitConstants[4]); // line circom 28
// end load src
Fr_copy(aux_dest,&expaux[0]);
}
{
PFrElement aux_dest = &lvar[2];
// load src
Fr_add(&expaux[0],&lvar[2],&circuitConstants[4]); // line circom 28
// end load src
Fr_copy(aux_dest,&expaux[0]);
}
Fr_lt(&expaux[0],&lvar[3],&circuitConstants[0]); // line circom 28
}
{
PFrElement aux_dest = &signalValues[mySignalStart + 0];
// load src
Fr_mul(&expaux[2],&circuitConstants[6],&signalValues[mySignalStart + 12]); // line circom 46
Fr_add(&expaux[1],&expaux[2],&signalValues[mySignalStart + 13]); // line circom 46
Fr_add(&expaux[0],&expaux[1],&signalValues[mySignalStart + 14]); // line circom 46
// end load src
Fr_copy(aux_dest,&expaux[0]);
}
{
PFrElement aux_dest = &signalValues[mySignalStart + 1];
// load src
Fr_mul(&expaux[2],&circuitConstants[6],&signalValues[mySignalStart + 13]); // line circom 47
Fr_add(&expaux[1],&signalValues[mySignalStart + 12],&expaux[2]); // line circom 47
Fr_add(&expaux[0],&expaux[1],&signalValues[mySignalStart + 14]); // line circom 47
// end load src
Fr_copy(aux_dest,&expaux[0]);
}
{
PFrElement aux_dest = &signalValues[mySignalStart + 2];
// load src
Fr_add(&expaux[1],&signalValues[mySignalStart + 12],&signalValues[mySignalStart + 13]); // line circom 48
Fr_mul(&expaux[2],&circuitConstants[0],&signalValues[mySignalStart + 14]); // line circom 48
Fr_add(&expaux[0],&expaux[1],&expaux[2]); // line circom 48
// end load src
Fr_copy(aux_dest,&expaux[0]);
}
for (uint i = 0; i < 3; i++){
uint index_subc = ctx->componentMemory[ctx_index].subcomponents[i];
if (index_subc != 0)release_memory_component(ctx,index_subc);
}
}

void Poseidon2Round_3_create(uint soffset,uint coffset,Circom_CalcWit* ctx,std::string componentName,uint componentFather){
ctx->componentMemory[coffset].templateId = 3;
ctx->componentMemory[coffset].templateName = "Poseidon2Round";
ctx->componentMemory[coffset].signalStart = soffset;
ctx->componentMemory[coffset].inputCounter = 6;
ctx->componentMemory[coffset].componentName = componentName;
ctx->componentMemory[coffset].idFather = componentFather;
ctx->componentMemory[coffset].subcomponents = new uint[1]{0};
}

void Poseidon2Round_3_run(uint ctx_index,Circom_CalcWit* ctx){
FrElement* circuitConstants = ctx->circuitConstants;
FrElement* signalValues = ctx->signalValues;
FrElement expaux[4];
FrElement lvar[4];
u64 mySignalStart = ctx->componentMemory[ctx_index].signalStart;
std::string myTemplateName = ctx->componentMemory[ctx_index].templateName;
std::string myComponentName = ctx->componentMemory[ctx_index].componentName;
u64 myFather = ctx->componentMemory[ctx_index].idFather;
u64 myId = ctx_index;
u32* mySubcomponents = ctx->componentMemory[ctx_index].subcomponents;
bool* mySubcomponentsParallel = ctx->componentMemory[ctx_index].subcomponentsParallel;
std::string* listOfTemplateMessages = ctx->listOfTemplateMessages;
uint sub_component_aux;
uint index_multiple_eq;
int cmp_index_ref_load = -1;
{
PFrElement aux_dest = &lvar[0];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[2]);
}
{
PFrElement aux_dest = &lvar[1];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[0]);
}
{
std::string new_cmp_name = "PowerFive_33_950";
PowerFive_1_create(mySignalStart+15,0+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[0] = 0+ctx_index+1;
}
{
PFrElement aux_dest = &lvar[2];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[2]);
}
{
PFrElement aux_dest = &lvar[3];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[2]);
}
Fr_lt(&expaux[0],&lvar[3],&circuitConstants[0]); // line circom 19
while(Fr_isTrue(&expaux[0])){
{
PFrElement aux_dest = &signalValues[mySignalStart + ((1 * Fr_toInt(&lvar[3])) + 9)];
// load src
Fr_add(&expaux[0],&signalValues[mySignalStart + ((1 * Fr_toInt(&lvar[3])) + 3)],&signalValues[mySignalStart + ((1 * Fr_toInt(&lvar[3])) + 6)]); // line circom 20
// end load src
Fr_copy(aux_dest,&expaux[0]);
}
{
PFrElement aux_dest = &lvar[3];
// load src
Fr_add(&expaux[0],&lvar[3],&circuitConstants[4]); // line circom 19
// end load src
Fr_copy(aux_dest,&expaux[0]);
}
Fr_lt(&expaux[0],&lvar[3],&circuitConstants[0]); // line circom 19
}
{
uint cmp_index_ref = 0;
{
PFrElement aux_dest = &ctx->signalValues[ctx->componentMemory[mySubcomponents[cmp_index_ref]].signalStart + 1];
// load src
// end load src
Fr_copy(aux_dest,&signalValues[mySignalStart + 9]);
}
// need to run sub component
ctx->componentMemory[mySubcomponents[cmp_index_ref]].inputCounter -= 1;
assert(!(ctx->componentMemory[mySubcomponents[cmp_index_ref]].inputCounter));
PowerFive_1_run(mySubcomponents[cmp_index_ref],ctx);
}
{
PFrElement aux_dest = &signalValues[mySignalStart + 12];
// load src
cmp_index_ref_load = 0;
cmp_index_ref_load = 0;
// end load src
Fr_copy(aux_dest,&ctx->signalValues[ctx->componentMemory[mySubcomponents[0]].signalStart + 0]);
}
{
PFrElement aux_dest = &lvar[3];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[4]);
}
Fr_lt(&expaux[0],&lvar[3],&circuitConstants[0]); // line circom 34
while(Fr_isTrue(&expaux[0])){
{
PFrElement aux_dest = &signalValues[mySignalStart + ((1 * Fr_toInt(&lvar[3])) + 12)];
// load src
// end load src
Fr_copy(aux_dest,&signalValues[mySignalStart + ((1 * Fr_toInt(&lvar[3])) + 9)]);
}
{
PFrElement aux_dest = &lvar[3];
// load src
Fr_add(&expaux[0],&lvar[3],&circuitConstants[4]); // line circom 34
// end load src
Fr_copy(aux_dest,&expaux[0]);
}
Fr_lt(&expaux[0],&lvar[3],&circuitConstants[0]); // line circom 34
}
{
PFrElement aux_dest = &signalValues[mySignalStart + 0];
// load src
Fr_mul(&expaux[2],&circuitConstants[6],&signalValues[mySignalStart + 12]); // line circom 46
Fr_add(&expaux[1],&expaux[2],&signalValues[mySignalStart + 13]); // line circom 46
Fr_add(&expaux[0],&expaux[1],&signalValues[mySignalStart + 14]); // line circom 46
// end load src
Fr_copy(aux_dest,&expaux[0]);
}
{
PFrElement aux_dest = &signalValues[mySignalStart + 1];
// load src
Fr_mul(&expaux[2],&circuitConstants[6],&signalValues[mySignalStart + 13]); // line circom 47
Fr_add(&expaux[1],&signalValues[mySignalStart + 12],&expaux[2]); // line circom 47
Fr_add(&expaux[0],&expaux[1],&signalValues[mySignalStart + 14]); // line circom 47
// end load src
Fr_copy(aux_dest,&expaux[0]);
}
{
PFrElement aux_dest = &signalValues[mySignalStart + 2];
// load src
Fr_add(&expaux[1],&signalValues[mySignalStart + 12],&signalValues[mySignalStart + 13]); // line circom 48
Fr_mul(&expaux[2],&circuitConstants[0],&signalValues[mySignalStart + 14]); // line circom 48
Fr_add(&expaux[0],&expaux[1],&expaux[2]); // line circom 48
// end load src
Fr_copy(aux_dest,&expaux[0]);
}
for (uint i = 0; i < 1; i++){
uint index_subc = ctx->componentMemory[ctx_index].subcomponents[i];
if (index_subc != 0)release_memory_component(ctx,index_subc);
}
}

void Poseidon2Permutation_4_create(uint soffset,uint coffset,Circom_CalcWit* ctx,std::string componentName,uint componentFather){
ctx->componentMemory[coffset].templateId = 4;
ctx->componentMemory[coffset].templateName = "Poseidon2Permutation";
ctx->componentMemory[coffset].signalStart = soffset;
ctx->componentMemory[coffset].inputCounter = 3;
ctx->componentMemory[coffset].componentName = componentName;
ctx->componentMemory[coffset].idFather = componentFather;
ctx->componentMemory[coffset].subcomponents = new uint[65]{0};
}

void Poseidon2Permutation_4_run(uint ctx_index,Circom_CalcWit* ctx){
FrElement* circuitConstants = ctx->circuitConstants;
FrElement* signalValues = ctx->signalValues;
FrElement expaux[4];
FrElement lvar[7];
u64 mySignalStart = ctx->componentMemory[ctx_index].signalStart;
std::string myTemplateName = ctx->componentMemory[ctx_index].templateName;
std::string myComponentName = ctx->componentMemory[ctx_index].componentName;
u64 myFather = ctx->componentMemory[ctx_index].idFather;
u64 myId = ctx_index;
u32* mySubcomponents = ctx->componentMemory[ctx_index].subcomponents;
bool* mySubcomponentsParallel = ctx->componentMemory[ctx_index].subcomponentsParallel;
std::string* listOfTemplateMessages = ctx->listOfTemplateMessages;
uint sub_component_aux;
uint index_multiple_eq;
int cmp_index_ref_load = -1;
{
PFrElement aux_dest = &lvar[0];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[0]);
}
{
std::string new_cmp_name = "rounds[0]";
Poseidon2Round_2_create(mySignalStart+393,1+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[0] = 1+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[1]";
Poseidon2Round_2_create(mySignalStart+420,5+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[1] = 5+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[2]";
Poseidon2Round_2_create(mySignalStart+447,9+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[2] = 9+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[3]";
Poseidon2Round_2_create(mySignalStart+474,13+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[3] = 13+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[4]";
Poseidon2Round_3_create(mySignalStart+501,17+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[4] = 17+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[5]";
Poseidon2Round_3_create(mySignalStart+520,19+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[5] = 19+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[6]";
Poseidon2Round_3_create(mySignalStart+539,21+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[6] = 21+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[7]";
Poseidon2Round_3_create(mySignalStart+558,23+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[7] = 23+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[8]";
Poseidon2Round_3_create(mySignalStart+577,25+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[8] = 25+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[9]";
Poseidon2Round_3_create(mySignalStart+596,27+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[9] = 27+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[10]";
Poseidon2Round_3_create(mySignalStart+615,29+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[10] = 29+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[11]";
Poseidon2Round_3_create(mySignalStart+634,31+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[11] = 31+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[12]";
Poseidon2Round_3_create(mySignalStart+653,33+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[12] = 33+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[13]";
Poseidon2Round_3_create(mySignalStart+672,35+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[13] = 35+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[14]";
Poseidon2Round_3_create(mySignalStart+691,37+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[14] = 37+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[15]";
Poseidon2Round_3_create(mySignalStart+710,39+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[15] = 39+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[16]";
Poseidon2Round_3_create(mySignalStart+729,41+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[16] = 41+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[17]";
Poseidon2Round_3_create(mySignalStart+748,43+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[17] = 43+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[18]";
Poseidon2Round_3_create(mySignalStart+767,45+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[18] = 45+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[19]";
Poseidon2Round_3_create(mySignalStart+786,47+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[19] = 47+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[20]";
Poseidon2Round_3_create(mySignalStart+805,49+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[20] = 49+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[21]";
Poseidon2Round_3_create(mySignalStart+824,51+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[21] = 51+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[22]";
Poseidon2Round_3_create(mySignalStart+843,53+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[22] = 53+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[23]";
Poseidon2Round_3_create(mySignalStart+862,55+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[23] = 55+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[24]";
Poseidon2Round_3_create(mySignalStart+881,57+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[24] = 57+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[25]";
Poseidon2Round_3_create(mySignalStart+900,59+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[25] = 59+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[26]";
Poseidon2Round_3_create(mySignalStart+919,61+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[26] = 61+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[27]";
Poseidon2Round_3_create(mySignalStart+938,63+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[27] = 63+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[28]";
Poseidon2Round_3_create(mySignalStart+957,65+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[28] = 65+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[29]";
Poseidon2Round_3_create(mySignalStart+976,67+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[29] = 67+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[30]";
Poseidon2Round_3_create(mySignalStart+995,69+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[30] = 69+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[31]";
Poseidon2Round_3_create(mySignalStart+1014,71+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[31] = 71+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[32]";
Poseidon2Round_3_create(mySignalStart+1033,73+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[32] = 73+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[33]";
Poseidon2Round_3_create(mySignalStart+1052,75+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[33] = 75+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[34]";
Poseidon2Round_3_create(mySignalStart+1071,77+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[34] = 77+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[35]";
Poseidon2Round_3_create(mySignalStart+1090,79+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[35] = 79+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[36]";
Poseidon2Round_3_create(mySignalStart+1109,81+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[36] = 81+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[37]";
Poseidon2Round_3_create(mySignalStart+1128,83+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[37] = 83+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[38]";
Poseidon2Round_3_create(mySignalStart+1147,85+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[38] = 85+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[39]";
Poseidon2Round_3_create(mySignalStart+1166,87+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[39] = 87+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[40]";
Poseidon2Round_3_create(mySignalStart+1185,89+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[40] = 89+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[41]";
Poseidon2Round_3_create(mySignalStart+1204,91+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[41] = 91+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[42]";
Poseidon2Round_3_create(mySignalStart+1223,93+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[42] = 93+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[43]";
Poseidon2Round_3_create(mySignalStart+1242,95+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[43] = 95+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[44]";
Poseidon2Round_3_create(mySignalStart+1261,97+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[44] = 97+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[45]";
Poseidon2Round_3_create(mySignalStart+1280,99+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[45] = 99+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[46]";
Poseidon2Round_3_create(mySignalStart+1299,101+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[46] = 101+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[47]";
Poseidon2Round_3_create(mySignalStart+1318,103+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[47] = 103+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[48]";
Poseidon2Round_3_create(mySignalStart+1337,105+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[48] = 105+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[49]";
Poseidon2Round_3_create(mySignalStart+1356,107+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[49] = 107+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[50]";
Poseidon2Round_3_create(mySignalStart+1375,109+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[50] = 109+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[51]";
Poseidon2Round_3_create(mySignalStart+1394,111+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[51] = 111+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[52]";
Poseidon2Round_3_create(mySignalStart+1413,113+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[52] = 113+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[53]";
Poseidon2Round_3_create(mySignalStart+1432,115+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[53] = 115+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[54]";
Poseidon2Round_3_create(mySignalStart+1451,117+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[54] = 117+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[55]";
Poseidon2Round_3_create(mySignalStart+1470,119+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[55] = 119+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[56]";
Poseidon2Round_3_create(mySignalStart+1489,121+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[56] = 121+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[57]";
Poseidon2Round_3_create(mySignalStart+1508,123+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[57] = 123+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[58]";
Poseidon2Round_3_create(mySignalStart+1527,125+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[58] = 125+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[59]";
Poseidon2Round_3_create(mySignalStart+1546,127+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[59] = 127+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[60]";
Poseidon2Round_2_create(mySignalStart+1565,129+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[60] = 129+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[61]";
Poseidon2Round_2_create(mySignalStart+1592,133+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[61] = 133+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[62]";
Poseidon2Round_2_create(mySignalStart+1619,137+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[62] = 137+ctx_index+1;
}
{
std::string new_cmp_name = "rounds[63]";
Poseidon2Round_2_create(mySignalStart+1646,141+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[63] = 141+ctx_index+1;
}
{
std::string new_cmp_name = "constants";
Poseidon2Constants_0_create(mySignalStart+201,0+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[64] = 0+ctx_index+1;
}
{
if (!Fr_isTrue(&circuitConstants[4])) std::cout << "Failed assert in template/function " << myTemplateName << " line 21. " <<  "Followed trace of components: " << ctx->getTrace(myId) << std::endl;
assert(Fr_isTrue(&circuitConstants[4]));
}
{
PFrElement aux_dest = &lvar[1];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[14]);
}
{
PFrElement aux_dest = &lvar[2];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[15]);
}
{
PFrElement aux_dest = &lvar[3];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[1]);
}
{
PFrElement aux_dest = &lvar[4];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[2]);
}
Fr_lt(&expaux[0],&lvar[4],&circuitConstants[0]); // line circom 35
while(Fr_isTrue(&expaux[0])){
{
PFrElement aux_dest = &signalValues[mySignalStart + ((0 + (1 * Fr_toInt(&lvar[4]))) + 6)];
// load src
// end load src
Fr_copy(aux_dest,&signalValues[mySignalStart + ((1 * Fr_toInt(&lvar[4])) + 3)]);
}
{
PFrElement aux_dest = &lvar[4];
// load src
Fr_add(&expaux[0],&lvar[4],&circuitConstants[4]); // line circom 35
// end load src
Fr_copy(aux_dest,&expaux[0]);
}
Fr_lt(&expaux[0],&lvar[4],&circuitConstants[0]); // line circom 35
}
{
PFrElement aux_dest = &lvar[4];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[2]);
}
Fr_lt(&expaux[0],&lvar[4],&circuitConstants[1]); // line circom 40
while(Fr_isTrue(&expaux[0])){
{
PFrElement aux_dest = &lvar[5];
// load src
Fr_lt(&expaux[1],&lvar[4],&circuitConstants[16]); // line circom 41
Fr_geq(&expaux[2],&lvar[4],&circuitConstants[17]); // line circom 41
Fr_lor(&expaux[0],&expaux[1],&expaux[2]); // line circom 41
// end load src
Fr_copy(aux_dest,&expaux[0]);
}
{
PFrElement aux_dest = &lvar[6];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[2]);
}
Fr_lt(&expaux[0],&lvar[6],&circuitConstants[0]); // line circom 45
while(Fr_isTrue(&expaux[0])){
{
uint cmp_index_ref = ((1 * Fr_toInt(&lvar[4])) + 0);
{
uint map_accesses_aux[1];
{
IOFieldDef *cur_def = &(ctx->templateInsId2IOSignalInfo[ctx->componentMemory[mySubcomponents[cmp_index_ref]].templateId].defs[1]);
{
uint map_index_aux[1];
map_index_aux[0]=Fr_toInt(&lvar[6]);
map_accesses_aux[0] = map_index_aux[0]*cur_def->size;
}
}
{
PFrElement aux_dest = &ctx->signalValues[ctx->componentMemory[mySubcomponents[cmp_index_ref]].signalStart + ctx->templateInsId2IOSignalInfo[ctx->componentMemory[mySubcomponents[cmp_index_ref]].templateId].defs[1].offset+map_accesses_aux[0]];
// load src
// end load src
Fr_copy(aux_dest,&signalValues[mySignalStart + (((3 * Fr_toInt(&lvar[4])) + (1 * Fr_toInt(&lvar[6]))) + 6)]);
}
// run sub component if needed
if(!(ctx->componentMemory[mySubcomponents[cmp_index_ref]].inputCounter -= 1)){
(*_functionTable[ctx->componentMemory[mySubcomponents[cmp_index_ref]].templateId])(mySubcomponents[cmp_index_ref],ctx);

}
}
}
{
uint cmp_index_ref = ((1 * Fr_toInt(&lvar[4])) + 0);
{
uint map_accesses_aux[1];
{
IOFieldDef *cur_def = &(ctx->templateInsId2IOSignalInfo[ctx->componentMemory[mySubcomponents[cmp_index_ref]].templateId].defs[2]);
{
uint map_index_aux[1];
map_index_aux[0]=Fr_toInt(&lvar[6]);
map_accesses_aux[0] = map_index_aux[0]*cur_def->size;
}
}
{
PFrElement aux_dest = &ctx->signalValues[ctx->componentMemory[mySubcomponents[cmp_index_ref]].signalStart + ctx->templateInsId2IOSignalInfo[ctx->componentMemory[mySubcomponents[cmp_index_ref]].templateId].defs[2].offset+map_accesses_aux[0]];
// load src
cmp_index_ref_load = 64;
cmp_index_ref_load = 64;
// end load src
Fr_copy(aux_dest,&ctx->signalValues[ctx->componentMemory[mySubcomponents[64]].signalStart + (((3 * Fr_toInt(&lvar[4])) + (1 * Fr_toInt(&lvar[6]))) + 0)]);
}
// run sub component if needed
if(!(ctx->componentMemory[mySubcomponents[cmp_index_ref]].inputCounter -= 1)){
(*_functionTable[ctx->componentMemory[mySubcomponents[cmp_index_ref]].templateId])(mySubcomponents[cmp_index_ref],ctx);

}
}
}
{
PFrElement aux_dest = &lvar[6];
// load src
Fr_add(&expaux[0],&lvar[6],&circuitConstants[4]); // line circom 45
// end load src
Fr_copy(aux_dest,&expaux[0]);
}
Fr_lt(&expaux[0],&lvar[6],&circuitConstants[0]); // line circom 45
}
{
PFrElement aux_dest = &lvar[6];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[2]);
}
Fr_lt(&expaux[0],&lvar[6],&circuitConstants[0]); // line circom 50
while(Fr_isTrue(&expaux[0])){
{
PFrElement aux_dest = &signalValues[mySignalStart + (((3 * (Fr_toInt(&lvar[4]) + 1)) + (1 * Fr_toInt(&lvar[6]))) + 6)];
// load src
cmp_index_ref_load = ((1 * Fr_toInt(&lvar[4])) + 0);
cmp_index_ref_load = ((1 * Fr_toInt(&lvar[4])) + 0);
// end load src
Fr_copy(aux_dest,&ctx->signalValues[ctx->componentMemory[mySubcomponents[((1 * Fr_toInt(&lvar[4])) + 0)]].signalStart + ctx->templateInsId2IOSignalInfo[ctx->componentMemory[mySubcomponents[((1 * Fr_toInt(&lvar[4])) + 0)]].templateId].defs[0].offset+(Fr_toInt(&lvar[6]))*ctx->templateInsId2IOSignalInfo[ctx->componentMemory[mySubcomponents[((1 * Fr_toInt(&lvar[4])) + 0)]].templateId].defs[0].size]);
}
{
PFrElement aux_dest = &lvar[6];
// load src
Fr_add(&expaux[0],&lvar[6],&circuitConstants[4]); // line circom 50
// end load src
Fr_copy(aux_dest,&expaux[0]);
}
Fr_lt(&expaux[0],&lvar[6],&circuitConstants[0]); // line circom 50
}
{
PFrElement aux_dest = &lvar[4];
// load src
Fr_add(&expaux[0],&lvar[4],&circuitConstants[4]); // line circom 40
// end load src
Fr_copy(aux_dest,&expaux[0]);
}
Fr_lt(&expaux[0],&lvar[4],&circuitConstants[1]); // line circom 40
}
{
PFrElement aux_dest = &lvar[4];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[2]);
}
Fr_lt(&expaux[0],&lvar[4],&circuitConstants[0]); // line circom 56
while(Fr_isTrue(&expaux[0])){
{
PFrElement aux_dest = &signalValues[mySignalStart + ((1 * Fr_toInt(&lvar[4])) + 0)];
// load src
// end load src
Fr_copy(aux_dest,&signalValues[mySignalStart + ((192 + (1 * Fr_toInt(&lvar[4]))) + 6)]);
}
{
PFrElement aux_dest = &lvar[4];
// load src
Fr_add(&expaux[0],&lvar[4],&circuitConstants[4]); // line circom 56
// end load src
Fr_copy(aux_dest,&expaux[0]);
}
Fr_lt(&expaux[0],&lvar[4],&circuitConstants[0]); // line circom 56
}
for (uint i = 0; i < 65; i++){
uint index_subc = ctx->componentMemory[ctx_index].subcomponents[i];
if (index_subc != 0)release_memory_component(ctx,index_subc);
}
}

void Poseidon2Hash_5_create(uint soffset,uint coffset,Circom_CalcWit* ctx,std::string componentName,uint componentFather){
ctx->componentMemory[coffset].templateId = 5;
ctx->componentMemory[coffset].templateName = "Poseidon2Hash";
ctx->componentMemory[coffset].signalStart = soffset;
ctx->componentMemory[coffset].inputCounter = 2;
ctx->componentMemory[coffset].componentName = componentName;
ctx->componentMemory[coffset].idFather = componentFather;
ctx->componentMemory[coffset].subcomponents = new uint[1]{0};
}

void Poseidon2Hash_5_run(uint ctx_index,Circom_CalcWit* ctx){
FrElement* circuitConstants = ctx->circuitConstants;
FrElement* signalValues = ctx->signalValues;
FrElement expaux[2];
FrElement lvar[2];
u64 mySignalStart = ctx->componentMemory[ctx_index].signalStart;
std::string myTemplateName = ctx->componentMemory[ctx_index].templateName;
std::string myComponentName = ctx->componentMemory[ctx_index].componentName;
u64 myFather = ctx->componentMemory[ctx_index].idFather;
u64 myId = ctx_index;
u32* mySubcomponents = ctx->componentMemory[ctx_index].subcomponents;
bool* mySubcomponentsParallel = ctx->componentMemory[ctx_index].subcomponentsParallel;
std::string* listOfTemplateMessages = ctx->listOfTemplateMessages;
uint sub_component_aux;
uint index_multiple_eq;
int cmp_index_ref_load = -1;
{
PFrElement aux_dest = &lvar[0];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[0]);
}
{
std::string new_cmp_name = "permutation";
Poseidon2Permutation_4_create(mySignalStart+3,0+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[0] = 0+ctx_index+1;
}
{
if (!Fr_isTrue(&circuitConstants[4])) std::cout << "Failed assert in template/function " << myTemplateName << " line 62. " <<  "Followed trace of components: " << ctx->getTrace(myId) << std::endl;
assert(Fr_isTrue(&circuitConstants[4]));
}
{
uint cmp_index_ref = 0;
{
PFrElement aux_dest = &ctx->signalValues[ctx->componentMemory[mySubcomponents[cmp_index_ref]].signalStart + 3];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[2]);
}
// run sub component if needed
if(!(ctx->componentMemory[mySubcomponents[cmp_index_ref]].inputCounter -= 1)){
Poseidon2Permutation_4_run(mySubcomponents[cmp_index_ref],ctx);

}
}
{
PFrElement aux_dest = &lvar[1];
// load src
// end load src
Fr_copy(aux_dest,&circuitConstants[2]);
}
Fr_lt(&expaux[0],&lvar[1],&circuitConstants[6]); // line circom 72
while(Fr_isTrue(&expaux[0])){
{
uint cmp_index_ref = 0;
{
PFrElement aux_dest = &ctx->signalValues[ctx->componentMemory[mySubcomponents[cmp_index_ref]].signalStart + ((1 * (Fr_toInt(&lvar[1]) + 1)) + 3)];
// load src
// end load src
Fr_copy(aux_dest,&signalValues[mySignalStart + ((1 * Fr_toInt(&lvar[1])) + 1)]);
}
// run sub component if needed
if(!(ctx->componentMemory[mySubcomponents[cmp_index_ref]].inputCounter -= 1)){
Poseidon2Permutation_4_run(mySubcomponents[cmp_index_ref],ctx);

}
}
{
PFrElement aux_dest = &lvar[1];
// load src
Fr_add(&expaux[0],&lvar[1],&circuitConstants[4]); // line circom 72
// end load src
Fr_copy(aux_dest,&expaux[0]);
}
Fr_lt(&expaux[0],&lvar[1],&circuitConstants[6]); // line circom 72
}
{
PFrElement aux_dest = &signalValues[mySignalStart + 0];
// load src
cmp_index_ref_load = 0;
cmp_index_ref_load = 0;
// end load src
Fr_copy(aux_dest,&ctx->signalValues[ctx->componentMemory[mySubcomponents[0]].signalStart + 1]);
}
for (uint i = 0; i < 1; i++){
uint index_subc = ctx->componentMemory[ctx_index].subcomponents[i];
if (index_subc != 0)release_memory_component(ctx,index_subc);
}
}

void Poseidon2ZK_6_create(uint soffset,uint coffset,Circom_CalcWit* ctx,std::string componentName,uint componentFather){
ctx->componentMemory[coffset].templateId = 6;
ctx->componentMemory[coffset].templateName = "Poseidon2ZK";
ctx->componentMemory[coffset].signalStart = soffset;
ctx->componentMemory[coffset].inputCounter = 3;
ctx->componentMemory[coffset].componentName = componentName;
ctx->componentMemory[coffset].idFather = componentFather;
ctx->componentMemory[coffset].subcomponents = new uint[1]{0};
}

void Poseidon2ZK_6_run(uint ctx_index,Circom_CalcWit* ctx){
FrElement* circuitConstants = ctx->circuitConstants;
FrElement* signalValues = ctx->signalValues;
FrElement expaux[1];
FrElement lvar[0];
u64 mySignalStart = ctx->componentMemory[ctx_index].signalStart;
std::string myTemplateName = ctx->componentMemory[ctx_index].templateName;
std::string myComponentName = ctx->componentMemory[ctx_index].componentName;
u64 myFather = ctx->componentMemory[ctx_index].idFather;
u64 myId = ctx_index;
u32* mySubcomponents = ctx->componentMemory[ctx_index].subcomponents;
bool* mySubcomponentsParallel = ctx->componentMemory[ctx_index].subcomponentsParallel;
std::string* listOfTemplateMessages = ctx->listOfTemplateMessages;
uint sub_component_aux;
uint index_multiple_eq;
int cmp_index_ref_load = -1;
{
std::string new_cmp_name = "hasher";
Poseidon2Hash_5_create(mySignalStart+3,0+ctx_index+1,ctx,new_cmp_name,myId);
mySubcomponents[0] = 0+ctx_index+1;
}
{
uint cmp_index_ref = 0;
{
PFrElement aux_dest = &ctx->signalValues[ctx->componentMemory[mySubcomponents[cmp_index_ref]].signalStart + 1];
// load src
// end load src
Fr_copy(aux_dest,&signalValues[mySignalStart + 0]);
}
// no need to run sub component
ctx->componentMemory[mySubcomponents[cmp_index_ref]].inputCounter -= 1;
assert(ctx->componentMemory[mySubcomponents[cmp_index_ref]].inputCounter > 0);
}
{
uint cmp_index_ref = 0;
{
PFrElement aux_dest = &ctx->signalValues[ctx->componentMemory[mySubcomponents[cmp_index_ref]].signalStart + 2];
// load src
// end load src
Fr_copy(aux_dest,&signalValues[mySignalStart + 1]);
}
// need to run sub component
ctx->componentMemory[mySubcomponents[cmp_index_ref]].inputCounter -= 1;
assert(!(ctx->componentMemory[mySubcomponents[cmp_index_ref]].inputCounter));
Poseidon2Hash_5_run(mySubcomponents[cmp_index_ref],ctx);
}
{
cmp_index_ref_load = 0;
cmp_index_ref_load = 0;
{{
Fr_eq(&expaux[0],&signalValues[mySignalStart + 2],&ctx->signalValues[ctx->componentMemory[mySubcomponents[0]].signalStart + 0]); // line circom 98
}}
if (!Fr_isTrue(&expaux[0])) std::cout << "Failed assert in template/function " << myTemplateName << " line 98. " <<  "Followed trace of components: " << ctx->getTrace(myId) << std::endl;
assert(Fr_isTrue(&expaux[0]));
}
for (uint i = 0; i < 1; i++){
uint index_subc = ctx->componentMemory[ctx_index].subcomponents[i];
if (index_subc != 0)release_memory_component(ctx,index_subc);
}
}

void run(Circom_CalcWit* ctx){
Poseidon2ZK_6_create(1,0,ctx,"main",0);
Poseidon2ZK_6_run(0,ctx);
}

