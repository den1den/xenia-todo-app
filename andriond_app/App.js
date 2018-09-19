import React from 'react';
import { Button, View, Text, StyleSheet, TextInput, Picker, FlatList, TouchableOpacity, Image,} from 'react-native';
import { StackNavigator } from 'react-navigation'; // Version can be specified in package.json
import CheckBox from 'react-native-checkbox';


class HomeScreen extends React.Component {
  static navigationOptions = {
    title: '', //HOME
  };
  render() {
    return (
      <View style={{ flex: 1, alignItems: 'center', justifyContent: 'center' }}>
        <Text>Home Screen xenia 2</Text>
        <View style={styles.buttonContainer}>
            <Button
                title="UNSCHEDULED"
                onPress={() => {
                    this.props.navigation.navigate('Unscheduled');
                }}
                color="#000"
            />
        </View>
        <View style={styles.buttonContainer}>
            <Button
                title="TODAY"
                onPress={() => {
                    this.props.navigation.navigate('Today');
                }}
                color="#000"
            />
        </View>
        <View style={styles.buttonContainer}>
            <Button
                title="DONE"
                onPress={() => {
                    this.props.navigation.navigate('Done');
                }}
                color="#000"
            />
        </View>
        <View style={styles.buttonContainer}>
            <Button
                title="ADD NEW TO DO"
                onPress={() => {
                    this.props.navigation.navigate('NewTodo');
                }}
                color="#000"
            />
            <TouchableOpacity style={styles.button} onPress={()=>{alert("you clicked me")}}>
                <Image source={require("./assets/addButton.png")}/>
            </TouchableOpacity>
        </View>
      </View>
    );
  }
}

class UnscheduledScreen extends React.Component {
    static navigationOptions = {
        title: 'UNSCHEDULED',
    };

    //haal data op
    state = {
        data: [{'name':''}]
    }
    componentDidMount() {
        fetch('http://10.0.2.2:8000/api/all-todos?format=json', { method: 'GET' })
            .then(response => response.json())
            .then(responseJson => {
                console.log(responseJson);
                this.setState({'data': responseJson, })
            })
            .catch((error) => {
                console.error(error);
            });
    }

    render() {
        return (
            <View style={{ flex: 1, alignItems: 'center', justifyContent: 'center' }}>
                <View style={{flex: 1, paddingTop:50}}>
                    {/*<Text>{(this.state.data[0].name)}</Text>*/}
                    <FlatList
                        data={this.state.data.filter((d) => d.status === "0" && d.completion_date === "0")}
                        renderItem={({item}) => <Text>{item.name}, {item.status}</Text>}
                        //keyExtractor={(item, index) => index}
                    />
                </View>
            </View>
        );
    }
}

class TodayScreen extends React.Component {
    static navigationOptions = {
        title: 'TODAY',
    };

    //haal data op
    state = {
        data: [{'name':''}]
    }
    componentDidMount() {
        fetch('http://10.0.2.2:8000/api/all-todos?format=json', { method: 'GET' })
            .then(response => response.json())
            .then(responseJson => {
                console.log(responseJson);
                this.setState({'data': responseJson, })
            })
            .catch((error) => {
                console.error(error);
            });
    }

    render() {
        return (
            <View style={{ flex: 1, alignItems: 'center', justifyContent: 'center' }}>
                <View style={{flex: 1, paddingTop:50}}>
                    {/*<Text>{(this.state.data[0].name)}</Text>*/}
                    <FlatList
                        data={this.state.data.filter((d) => d.status === "0" && d.completion_date === "1")}
                        renderItem={({item}) => <Text>{item.name}, {item.status}</Text>}
                        //keyExtractor={(item, index) => index}
                    />
                </View>
            </View>
        );
    }
}

class DoneScreen extends React.Component {
    static navigationOptions = {
        title: 'DONE',
    };

    //haal data op
    state = {
        data: [{'name':''}]
    }
    componentDidMount() {
        fetch('http://10.0.2.2:8000/api/all-todos?format=json', { method: 'GET' })
            .then(response => response.json())
            .then(responseJson => {
                console.log(responseJson);
                this.setState({'data': responseJson, })
            })
            .catch((error) => {
                console.error(error);
            });
    }

    render() {
        return (
            <View style={{ flex: 1, alignItems: 'center', justifyContent: 'center' }}>
                <View style={{flex: 1, paddingTop:50}}>
                    {/*<Text>{(this.state.data[0].name)}</Text>*/}
                    <FlatList
                        data={this.state.data.filter((d) => d.status === "1")}
                        renderItem={({item}) => <Text>{item.updated_at}, {item.name}</Text>}
                        //keyExtractor={(item, index) => index}
                    />
                </View>
            </View>
        );
    }
}

class NewTodoScreen extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            state: 'Anywhere'
        }
    }

    static navigationOptions = {
        title: 'ADD NEW',
    };
    render() {
        return (
            <View style={{ flex: 1, alignItems: 'center', justifyContent: 'center' }}>
                <Text>What</Text>
                <View style={styles.inputContainer}>
                    <TextInput
                        style={styles.textInput}
                        //onChangeText={this.changeTextHandler}
                        //onSubmitEditing={this.addTask}
                        //value={this.state.text}
                        placeholder="Add new"
                        returnKeyType="done"
                        returnKeyLabel="done"
                    />
                </View>
                <Text>Where</Text>
                <View style={styles.pickerContainer}>
                    <Picker
                        selectedValue={this.state.location}
                        style={styles.picker}
                        onValueChange={(itemValue, itemIndex) => this.setState({location: itemValue})}>
                        <Picker.Item label="Anywhere" value="Anywhere" />
                        <Picker.Item label="Wemos 1" value="Wemos1" />
                        <Picker.Item label="Wemos 2" value="Wemos2" />
                        <Picker.Item label="Wemos 3" value="Wemos3" />
                    </Picker>
                </View>
                <CheckBox
                    label='Today'
                    //title='Click Here'
                    checked={this.state.checked}
                />
                <CheckBox
                    label='Important'
                    //title='Click Here'
                    checked={this.state.checked}
                />
                <View style={styles.buttonContainer}>
                    <Button
                        title="ADD TO DO"
                        // onPress={() => {
                        //     this.props.navigation.navigate('Unscheduled');
                        // }}
                        onPress={() => this.props.navigation.goBack()}
                        color="#000"
                    />
                </View>
            </View>
        );
    }
}

const RootStack = StackNavigator(
  {
    Home: {
      screen: HomeScreen,
    },
    Unscheduled: {
          screen: UnscheduledScreen,
    },
    Today: {
      screen: TodayScreen,
    },
    Done: {
      screen: DoneScreen,
    },
    NewTodo: {
      screen: NewTodoScreen,
    },
  },
  {
    initialRouteName: 'Home',
    navigationOptions: {
      headerStyle: {
        backgroundColor: '#fff',
        position: 'absolute',
        height: 50,
        top: 0,
        left: 0,
        right: 0,
        },
        headerTintColor: '#000',
        headerTitleStyle: {
          fontWeight: 'bold',
          textAlign: 'center',
          alignSelf: 'center',
          marginHorizontal: 0,
          width: '100%',
          flex: 1,
        },
        headerRight: (<View></View>)
      },
  }
);

export default class App extends React.Component {
  render() {
    return <RootStack />;
  }
}

const styles = StyleSheet.create({

 buttonContainer: {
   margin: 20,
 },

 button: {
   margin: 20,
 },

 textInput: {
     //paddingRight: 50,
     //paddingLeft: 50,
 },

 inputContainer: {
     borderWidth: 1,
     width: '90%',
 },

 picker: {
     //width: '90%',
 },

 pickerContainer: {
    borderWidth: 1,
    width:'90%',
 },

})


//import React, { Component } from 'react';
//import { Alert, AppRegistry, Button, StyleSheet, View, Text } from 'react-native';
//
//export default class ButtonBasics extends Component {
//  _onPressButton() {
//    Alert.alert('You tapped the button!')
//  }
//
//  render() {
//    return (
//      <View style={styles.container}>
//		<Text style={styles.h1}>MENU</Text>
//
//
//		<View style={styles.buttonContainer}>
//          <Button
//            onPress={this._onPressButton}
//            title="UNSCHEDULED"
//			color="#000"
//			/>
//        </View>
//
//		<View style={styles.buttonContainer}>
//          <Button
//            onPress={this._onPressButton}
//            title="TODAY"
//			color="#000"
//          />
//        </View>
//		<View style={styles.buttonContainer}>
//          <Button
//            onPress={this._onPressButton}
//            title="DONE"
//			color="#000"
//          />
//        </View>
//      </View>
//    );
//  }
//}
//
//const styles = StyleSheet.create({
//  container: {
//   flex: 1,
//   flexDirection: 'column',
//   justifyContent: 'flex-start',
//  },
//	h1:{
//		fontSize: 50,
//		textAlign: 'center',
//	},
//	h2:{
//		fontSize: 30,
//		textAlign: 'left',
//	},
//  buttonContainer: {
//    margin: 20,
//  },
//  alternativeLayoutButtonContainer: {
//    margin: 20,
//    flexDirection: 'row',
//    justifyContent: 'space-between'
//  }
//})